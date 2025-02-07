// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "time_stamp_count.hpp"
#include "application.hpp"
#include "logger.hpp"
#include "thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <bit>
#include <iterator>

namespace tt {

using namespace std::chrono_literals;

std::string format_engineering(hires_utc_clock::duration duration)
{
    if (duration >= 1s) {
        return fmt::format("{:.3g} s ", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return fmt::format("{:.3g} ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return fmt::format("{:.3g} us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return fmt::format("{:.3g} ns", static_cast<double>(duration / 1ns));
    }
}

[[nodiscard]] hires_utc_clock::time_point hires_utc_clock::now(time_stamp_count &tsc) noexcept
{
    auto shortest_diff = std::numeric_limits<uint64_t>::max();
    time_stamp_count shortest_tsc;
    hires_utc_clock::time_point shortest_tp;

    // With three samples gathered on the same CPU we should
    // have a TSC/UTC/TSC combination that was run inside a single time-slice.
    for (auto i = 0; i != 10; ++i) {
        ttlet tmp_tsc1 = time_stamp_count::now();
        ttlet tmp_tp = hires_utc_clock::now();
        ttlet tmp_tsc2 = time_stamp_count::now();

        if (tmp_tsc1.cpu_id() != tmp_tsc2.cpu_id()) {
            tt_log_fatal("CPU Switch detected during get_sample(), which should never happen");
        }

        if (tmp_tsc1.count() > tmp_tsc2.count()) {
            tt_log_warning("TSC skipped backwards");
            continue;
        }

        ttlet diff = tmp_tsc2.count() - tmp_tsc1.count();

        if (diff < shortest_diff) {
            shortest_diff = diff;
            shortest_tp = tmp_tp;
            shortest_tsc = tmp_tsc1 + (diff / 2);
        }
    }

    if (shortest_diff == std::numeric_limits<uint64_t>::max()) {
        tt_log_fatal("Unable to get TSC sample.");
    }

    tsc = shortest_tsc;
    return shortest_tp;
}

[[nodiscard]] hires_utc_clock::time_point hires_utc_clock::make(time_stamp_count const &tsc) noexcept
{
    auto i = tsc.cpu_id();
    if (i >= 0) {
        ttlet tsc_epoch = tsc_epochs[i].load(std::memory_order::relaxed);
        if (tsc_epoch != hires_utc_clock::time_point{}) {
            return tsc_epoch + tsc.time_since_epoch();
        }
    }

    // Fallback.
    ttlet ref_tp = hires_utc_clock::now();
    ttlet ref_tsc = time_stamp_count::now();
    ttlet diff_ns = ref_tsc.time_since_epoch() - tsc.time_since_epoch();
    return ref_tp - diff_ns;
}

void hires_utc_clock::subsystem_proc_frequency_calibration(std::stop_token stop_token) noexcept
{
    // Calibrate the TSC frequency to within 1 ppm.
    // A 1s measurement already brings is to about 1ppm. We are
    // going to be taking average of the IQR of 11 samples, just
    // in case there are UTC clock adjustment made during the measurement.

    std::array<uint64_t, 16> frequencies;
    for (auto i = 0; i != frequencies.size();) {
        ttlet f = time_stamp_count::measure_frequency(1s);
        if (f != 0) {
            frequencies[i] = f;
            ++i;
        }

        if (stop_token.stop_requested()) {
            return;
        }
    }
    std::ranges::sort(frequencies);
    ttlet iqr_size = frequencies.size() / 2;
    ttlet iqr_first = std::next(frequencies.cbegin(), frequencies.size() / 4);
    ttlet iqr_last = std::next(iqr_first, iqr_size);
    ttlet frequency = std::accumulate(iqr_first, iqr_last, uint64_t{0}) / iqr_size;

    tt_log_info("Accurate measurement of TSC frequency result is {} Hz", frequency);
    time_stamp_count::set_frequency(frequency);
}

static void advance_cpu_thread_mask(uint64_t const &process_cpu_mask, uint64_t &thread_cpu_mask)
{
    tt_axiom(std::popcount(process_cpu_mask) > 0);
    tt_axiom(std::popcount(thread_cpu_mask) == 1);

    do {
        if ((thread_cpu_mask <<= 1) == 0) {
            thread_cpu_mask = 1;
        }

    } while ((process_cpu_mask & thread_cpu_mask) == 0);
}

void hires_utc_clock::subsystem_proc(std::stop_token stop_token) noexcept
{
    set_thread_name("hires_utc_clock");
    subsystem_proc_frequency_calibration(stop_token);

    ttlet process_cpu_mask = process_affinity_mask();

    size_t next_cpu = 0;
    while (!stop_token.stop_requested()) {
        ttlet current_cpu = advance_thread_affinity(next_cpu);

        std::this_thread::sleep_for(100ms);
        ttlet lock = std::scoped_lock(hires_utc_clock::mutex);

        time_stamp_count tsc;
        ttlet tp = hires_utc_clock::now(tsc);
        tt_axiom(tsc.cpu_id() == narrow_cast<ssize_t>(current_cpu));

        tsc_epochs[current_cpu].store(tp - tsc.time_since_epoch(), std::memory_order::relaxed);        
    }
}

[[nodiscard]] bool hires_utc_clock::init_subsystem() noexcept
{
    hires_utc_clock::subsystem_thread = std::jthread{subsystem_proc};
    return true;
}

void hires_utc_clock::deinit_subsystem() noexcept
{
    if (hires_utc_clock::subsystem_thread.joinable()) {
        hires_utc_clock::subsystem_thread.request_stop();
        hires_utc_clock::subsystem_thread.join();
    }
}

bool hires_utc_clock::start_subsystem() noexcept
{
    return tt::start_subsystem(
        hires_utc_clock::subsystem_is_running, false, hires_utc_clock::init_subsystem, hires_utc_clock::deinit_subsystem);
}

void hires_utc_clock::stop_subsystem() noexcept
{
    return tt::stop_subsystem(hires_utc_clock::subsystem_is_running, false, hires_utc_clock::deinit_subsystem);
}

} // namespace tt
