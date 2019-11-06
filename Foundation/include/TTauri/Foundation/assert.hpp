// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/debugger.hpp"
#include  <exception>

namespace TTauri {

void assertIsLogged(const char *source_file, int source_line, const char *message);

#define AI_SKIP 'S'
#define AI_LOG 'L'
#define AI_TERMINATE 'T'
#define AI_ASSUME 'A'

#if defined(NDEBUG)
#define OPTIONAL_ASSERT_IMPLEMENTATION AI_SKIP
#define REVIEW_ASSERT_IMPLEMENTATION AI_LOG
#define REQUIRED_ASSERT_IMPLEMENTATION AI_TERMINATE
#define AXIOM_ASSERT_IMPLEMENTATION AI_ASSUME
#else
#define OPTIONAL_ASSERT_IMPLEMENTATION AI_TERMINATE
#define REVIEW_ASSERT_IMPLEMENTATION AI_TERMINATE
#define REQUIRED_ASSERT_IMPLEMENTATION AI_TERMINATE
#define AXIOM_ASSERT_IMPLEMENTATION AI_TERMINATE
#endif

/*! Check if the assert expression yields a boolean.
 */
#define ttauri_assert_impl_skip(x) static_assert(sizeof(x) == 1)

/*! Log an assertion.
 */
#define ttauri_assert_impl_log(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsLogged(__FILE__, __LINE__, #x);\
        }\
    } while (false)

/*! Terminate on assertion.
 */
#define ttauri_assert_impl_terminate(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            debugger_break;\
            std::terminate();\
        }\
    } while (false)

/*! Treat an assertion as an assumption.
 */
#define ttauri_assert_impl_assume(x) ttauri_assume(x);

#if OPTIONAL_ASSERT_IMPLEMENTATION == AI_SKIP
#define optional_assert(x) ttauri_assert_impl_skip(x)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_LOG
#define optional_assert(x) ttauri_assert_impl_log(x)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define optional_assert(x) ttauri_assert_impl_terminate(x)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_ASSUME
#define optional_assert(x) ttauri_assert_impl_assume(x)
#else
#error "Optional assert implementation not set."
#endif

#if REVIEW_ASSERT_IMPLEMENTATION == AI_SKIP
#define review_assert(x) ttauri_assert_impl_skip(x)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_LOG
#define review_assert(x) ttauri_assert_impl_log(x)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define review_assert(x) ttauri_assert_impl_terminate(x)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_ASSUME
#define review_assert(x) ttauri_assert_impl_assume(x)
#else
#error "Review assert implementation not set."
#endif

#if REQUIRED_ASSERT_IMPLEMENTATION == AI_SKIP
#define required_assert(x) ttauri_assert_impl_skip(x)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_LOG
#define required_assert(x) ttauri_assert_impl_log(x)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define required_assert(x) ttauri_assert_impl_terminate(x)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_ASSUME
#define required_assert(x) ttauri_assert_impl_assume(x)
#else
#error "Required assert implementation not set."
#endif

#if AXIOM_ASSERT_IMPLEMENTATION == AI_SKIP
#define axiom_assert(x) ttauri_assert_impl_skip(x)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_LOG
#define axiom_assert(x) ttauri_assert_impl_log(x)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define axiom_assert(x) ttauri_assert_impl_terminate(x)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_ASSUME
#define axiom_assert(x) ttauri_assert_impl_assume(x)
#else
#error "Axiom assert implementation not set."
#endif


#define no_default debugger_break; std::terminate();
#define not_implemented debugger_break; std::terminate();
#define ttauri_overflow debugger_break; std::terminate();

}