// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "unfair_mutex.hpp"
#include <atomic>
#include <vector>
#include <functional>
#include <bit>
#include <type_traits>
#include <mutex>

namespace tt {
namespace detail {

/** The status of the system, as an atomic value.
 * The status of the system is used as a bit field, so that multiple status flags can
 * be checked with a single read.
 *
 * The system status should only be written to when holding the system_status_mutex.
 */
enum class system_status_type { not_started, running, shutdown };

inline system_status_type system_status = system_status_type::not_started;

/** A list of deinit function to be called on shutdown.
 */
inline std::vector<void (*)()> subsystem_deinit_list;

/** Mutex to be held when writing to system_status or accessing system_status_deinit_list.
 * The system status is also an atomic variable so that reads on system_status
 * without holding the mutex is still possible.
 */
inline unfair_mutex subsystem_mutex;

template<typename T, typename InitFunc>
tt_no_inline T
start_subsystem(std::atomic<T> &check_variable, T off_value, InitFunc const &init_function, void (*deinit_function)())
{
    ttlet lock = std::scoped_lock(subsystem_mutex);

    auto old_value = check_variable.load(std::memory_order::acquire);
    if (old_value != off_value) {
        // In the short time before the lock the subsystem became available.
        return old_value;
    }

    if (system_status != system_status_type::running) {
        // Only when the system is running can subsystems be started.
        // otherwise they have to run in degraded mode.
        return off_value;
    }

    auto new_value = init_function();

    if (new_value != off_value) {
        subsystem_deinit_list.emplace_back(deinit_function);
        check_variable.store(new_value, std::memory_order::release);
    }

    return new_value;
}

} // namespace detail

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @param check_variable The variable to check before initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
template<typename T, typename InitFunc>
T start_subsystem(std::atomic<T> &check_variable, T off_value, InitFunc const &init_function, void (*deinit_function)())
{
    auto old_value = check_variable.load(std::memory_order::acquire);
    if (old_value == off_value) {
        [[unlikely]] return detail::start_subsystem(check_variable, off_value, init_function, deinit_function);
    } else {
        [[likely]] return old_value;
    }
}

/** Stop a sub-system.
 * De-initialize a subsystem.
 *
 * This will also unregister the deinit function to be called on system shutdown.
 *
 * @param check_variable The variable to check before de-initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param deinit_function the deinit function to call.
 */
template<typename T>
void stop_subsystem(std::atomic<T> &check_variable, T off_value, void (*deinit_function)())
{
    ttlet lock = std::scoped_lock(detail::subsystem_mutex);

    auto old_value = check_variable.exchange(off_value, std::memory_order::acquire);
    if (old_value == off_value) {
        return;
    }

    std::erase(detail::subsystem_deinit_list, deinit_function);

    //auto it = std::ranges::remove(detail::subsystem_deinit_list, deinit_function);
    //detail::subsystem_deinit_list.erase(it);

    return deinit_function();
}

/** Start the system.
 * Subsystems will only initialize once the system is started.
 */
inline void start_system() noexcept
{
    ttlet lock = std::scoped_lock(detail::subsystem_mutex);
    detail::system_status = detail::system_status_type::running;
}

/** Shutdown the system.
 * This will shutdown all the registered deinit functions.
 *
 * Any attempts at registering deinit functions after this call
 * will fail and the deinit function will be called directly.
 */
inline void shutdown_system() noexcept
{
    detail::subsystem_mutex.lock();
    detail::system_status = detail::system_status_type::shutdown;

    while (!detail::subsystem_deinit_list.empty()) {
        auto deinit = std::move(detail::subsystem_deinit_list.back());
        detail::subsystem_deinit_list.pop_back();

        detail::subsystem_mutex.unlock();
        deinit();
        detail::subsystem_mutex.lock();
    }
    detail::subsystem_mutex.unlock();
}

} // namespace tt
