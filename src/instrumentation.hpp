/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

#pragma once

#include <atomic>

#include <ovni.h>
#include "base_instr.hpp"


/**
 * Marker colors values of the default Paraver color palette
 */
enum mark_color : int64_t {
    MARK_COLOR_BLACK = 0,
    MARK_COLOR_BLUE = 1,
    MARK_COLOR_LIGHT_GRAY = 2,
    MARK_COLOR_RED = 3,
    MARK_COLOR_GREEN = 4,
    MARK_COLOR_YELLOW = 5,
    MARK_COLOR_ORANGE = 6,
    MARK_COLOR_PURPLE = 7,
    MARK_COLOR_CYAN = 8,
    MARK_COLOR_MAGENTA = 9,
    MARK_COLOR_LIGHT_GREEN = 10,
    MARK_COLOR_PINK = 11,
    MARK_COLOR_TEAL = 12,
    MARK_COLOR_GRAY = 13,
    MARK_COLOR_LAVENDER = 14,
    MARK_COLOR_BROWN = 15,
    MARK_COLOR_LIGHT_YELLOW = 16,
    MARK_COLOR_MAROON = 17,
    MARK_COLOR_MINT = 18,
    MARK_COLOR_OLIVE = 19,
    MARK_COLOR_PEACH = 20,
    MARK_COLOR_NAVY = 21,
    MARK_COLOR_BRIGHT_BLUE = 22
};

/**
 * Marker type defines the group a set of markers are from.
 * In our case, e.g. MARK_TYPE_RUNTIME are all markers for the runtime perspective.
 */
enum mark_type : int32_t {
    MARK_TYPE_RUNTIME = 0,
    MARK_TYPE_WORKER = 1
};

/**
 * 
 */
#ifdef USE_INSTRUMENTATION

    // atomic counter of the rank called
    std::atomic<int> rank_counter(0);

    // this boolean is needed if something other than ovni is called.
    #define INSTRUMENTATION_ACTIVE true    

    #define INSTRUMENTATION_START() instrumentation_init_proc();  instrumentation_init_thread(rank_counter.fetch_add(1)); 
    #define INSTRUMENTATION_END() instrumentation_end()

    #define INSTRUMENTATION_INIT_PROC() instrumentation_init_proc()

    #define INSTRUMENTATION_PROC_END() ovni_proc_fini()

    #define INSTRUMENTATION_INIT_THREAD() instrumentation_init_thread(rank_counter.fetch_add(1))

    #define INSTRUMENTATION_THREAD_END() instrumentation_thread_end(); ovni_thread_free()

    #define INSTRUMENTATION_THREAD_ISREADY() ovni_thread_isready()

    // markers
    #define INSTRUMENTATION_MARK_TYPE(type, flag, title) ovni_mark_type(type, flag, title)

    #define INSTRUMENTATION_MARK_LABEL(type, value, label) ovni_mark_label(type, value, label)

    #define INSTRUMENTATION_MARK_PUSH(type, value) ovni_mark_push(type, value)

    #define INSTRUMENTATION_MARK_POP(type, value) ovni_mark_pop(type, value)

    #define INSTRUMENTATION_MARK_SET(type, value) ovni_mark_set(type, value)


#else   /* No instrumentation (void) */

    #define INSTRUMENTATION_ACTIVE false

    #define INSTRUMENTATION_START()

    #define INSTRUMENTATION_END()

    #define INSTRUMENTATION_INIT_PROC()

    #define INSTRUMENTATION_PROC_END()

    #define INSTRUMENTATION_INIT_THREAD()

    #define INSTRUMENTATION_THREAD_END()

    #define INSTRUMENTATION_THREAD_ISREADY() false

    // markers
    #define INSTRUMENTATION_MARK_TYPE(type, flag, title) (void)(type); (void)(flag); (void)(title)

    #define INSTRUMENTATION_MARK_LABEL(type, value, label) (void)(type); (void)(value); (void)(label)

    #define INSTRUMENTATION_MARK_PUSH(type, value) (void)(type); (void)(value)

    #define INSTRUMENTATION_MARK_POP(type, value) (void)(type); (void)(value)

    #define INSTRUMENTATION_MARK_SET(type, value) (void)(type); (void)(value)


#endif  /* USE_INSTRUMENTATION */