/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file detectr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Baumann
 * @date 17/12/2024
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
    MARK_COLOR_BLUE,
    MARK_COLOR_LIGHT_GRAY,
    MARK_COLOR_RED,
    MARK_COLOR_GREEN,
    MARK_COLOR_YELLOW,
    MARK_COLOR_ORANGE,
    MARK_COLOR_PURPLE,
    MARK_COLOR_CYAN,
    MARK_COLOR_MAGENTA,
    MARK_COLOR_LIGHT_GREEN,
    MARK_COLOR_PINK,
    MARK_COLOR_TEAL,
    MARK_COLOR_GRAY,
    MARK_COLOR_LAVENDER,
    MARK_COLOR_BROWN,
    MARK_COLOR_LIGHT_YELLOW,
    MARK_COLOR_MAROON,
    MARK_COLOR_MINT,
    MARK_COLOR_OLIVE,
    MARK_COLOR_PEACH,
    MARK_COLOR_NAVY,
    MARK_COLOR_BRIGHT_BLUE
};

/**
 * 
 */
#if defined(INSTRUMENTATION_TASKS) || defined(INSTRUMENTATION_THREADS)
    // debug printing method. Can be enable with the ENABLE_DEBUG flag included.
    // NOT THREAD SAFE: will sometimes lead to segmentation fault. Use it on small examples.
    #ifdef ENABLE_DEBUG
        #define debug_print(fmt, ...) \
            printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
    #else
        #define debug_print(fmt, ...)
    #endif

    // atomic counter of how many tasks got created
    std::atomic<int> ntasks_counter(0);

    // keep track of the main thread as this one has to be free'd when instr_end is called
    pid_t main_TID;

    // this boolean is needed if something other than DetectR has to be called.
    #define INSTRUMENTATION_ACTIVE true    

    // ovni proc methods
    #define INSTRUMENTATION_START()         \
        debug_print("instr_start (TID: %d)", get_tid());       \
        main_TID = get_tid();               \
        instrumentation_init_proc(0, 1);    \
        ovni_thread_require("taskr", "1.0.0")
    
    #define INSTRUMENTATION_END()                                               \
        debug_print("instr_end (TID: %d)", get_tid());                          \
        ovni_attr_set_double("taskr.ntasks", (double) ntasks_counter.load());   \
        instrumentation_end()

    // ovni thread methods
    #define INSTRUMENTATION_THREAD_INIT()                                           \
        debug_print("instr_thread_init with isready: %d (TID: %d)", ovni_thread_isready(), get_tid()); \
        if(!ovni_thread_isready()) {                                                \
            instrumentation_init_thread();                                          \
        }

    #define INSTRUMENTATION_THREAD_END()                                            \
        debug_print("instr_thread_end with isready: %d (TID: %d)", ovni_thread_isready(), get_tid());  \
        if(ovni_thread_isready() && !(main_TID == get_tid())) {                                                 \
            instrumentation_thread_end();                                           \
            ovni_thread_free();                                                     \
        }

#else   /* No instrumentation (void) */

    #define INSTRUMENTATION_ACTIVE false

    // ovni proc methods
    #define INSTRUMENTATION_START()

    #define INSTRUMENTATION_END()

    // ovni thread methods
    #define INSTRUMENTATION_THREAD_INIT()

    #define INSTRUMENTATION_THREAD_END()
    
#endif

    

/**
 *  task marker methods
 */
#ifdef INSTRUMENTATION_TASKS
    #define INSTRUMENTATION_TASK_MARK_TYPE(chan_type)           \
        debug_print("instr_task_type_set: (TID: %d)", get_tid());   \
        ovni_attr_set_double("taskr.chan_type", (double) chan_type)
    
    #define INSTRUMENTATION_TASK_INIT()            \
        debug_print("instr_task_init: (TID: %d)", get_tid());   \
        ntasks_counter++

    #define INSTRUMENTATION_TASK_ADD(labelid, label)           \
        task_marker_map.add(labelid, label);                       \
        debug_print("instr_taskr_create: (TID: %d)", get_tid())

    #define INSTRUMENTATION_TASK_SET(taskid, idx)            \
        debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int) taskid, (int) idx, get_tid());   \
        task_marker_map.set(taskid, idx)

    #define INSTRUMENTATION_TASK_PUSH(taskid, idx)            \
        debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int) taskid, (int) idx, get_tid());   \
        task_marker_map.push(taskid, idx)

    #define INSTRUMENTATION_TASK_POP(taskid, idx)            \
        debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int) taskid, (int) idx, get_tid());   \
        task_marker_map.pop(taskid, idx)
#else
    #define INSTRUMENTATION_TASK_MARK_TYPE(chan_type) (void)(chan_type)

    #define INSTRUMENTATION_TASK_INIT()

    #define INSTRUMENTATION_TASK_ADD(labelid, label) -1; (void)(labelid); (void)(label)

    #define INSTRUMENTATION_TASK_SET(taskid, idx) (void)(taskid); (void)(idx)

    #define INSTRUMENTATION_TASK_PUSH(taskid, idx) (void)(taskid); (void)(idx)

    #define INSTRUMENTATION_TASK_POP(taskid, idx) (void)(taskid); (void)(idx)
#endif

/**
 * ovni thread marker methods
 */
#ifdef INSTRUMENTATION_THREADS
    #define INSTRUMENTATION_MARKER_INIT(flag)  \
        debug_print("instr_marker_init (TID: %d)", get_tid());   \
        ovni_mark_type(0, flag, "DetectR Thread Markers");

    #define INSTRUMENTATION_MARKER_ADD(labelid, label)  \
        thread_marker_map.add(labelid, label);                    \
        debug_print("instr_marker_add (TID: %d)", get_tid())   \

    #define INSTRUMENTATION_MARKER_SET(idx)  \
        debug_print("instr_marker_set idx: %ld (TID: %d)", idx, get_tid());   \
        thread_marker_map.set(idx);

    #define INSTRUMENTATION_MARKER_PUSH(idx)  \
        debug_print("instr_marker_push (TID: %d)", get_tid());   \
        thread_marker_map.push(idx);

    #define INSTRUMENTATION_MARKER_POP(idx)  \
        debug_print("instr_marker_pop (TID: %d)", get_tid());   \
        thread_marker_map.pop(idx);

    // ovni marker methods (vanilla) (only used for performance comparisons)
    #define INSTRUMENTATION_VMARKER_TYPE(flag, title)    \
        ovni_mark_type(0, flag, title)

    #define INSTRUMENTATION_VMARKER_LABEL(value, label)  \
        ovni_mark_label(0, value, label)

    #define INSTRUMENTATION_VMARKER_SET(value)   \
        debug_print("instr_marker_set (TID: %d)", get_tid());   \
        ovni_mark_set(0, value)
    
    #define INSTRUMENTATION_VMARKER_PUSH(value)  \
        debug_print("instr_marker_push (TID: %d)", get_tid());   \
        ovni_mark_push(0, value)

    #define INSTRUMENTATION_VMARKER_POP(value)   \
        debug_print("instr_marker_pop (TID: %d)", get_tid());   \
        ovni_mark_pop(0, value)
#else
    #define INSTRUMENTATION_MARKER_INIT(flag) (void)(flag)

    #define INSTRUMENTATION_MARKER_ADD(value, label) -1; (void)(value); (void)(label)

    #define INSTRUMENTATION_MARKER_SET(idx) (void)(idx)

    #define INSTRUMENTATION_MARKER_PUSH(idx) (void)(idx)

    #define INSTRUMENTATION_MARKER_POP(idx) (void)(idx)

    // ovni marker methods (vanilla) (only used for performance comparisons)
    #define INSTRUMENTATION_VMARKER_TYPE(flag, title) (void)(flag); (void)(title)

    #define INSTRUMENTATION_VMARKER_LABEL(value, label) (void)(value); (void)(label)

    #define INSTRUMENTATION_VMARKER_SET(value) (void)(value)

    #define INSTRUMENTATION_VMARKER_PUSH(value) (void)(value)

    #define INSTRUMENTATION_VMARKER_POP(value) (void)(value)
#endif
