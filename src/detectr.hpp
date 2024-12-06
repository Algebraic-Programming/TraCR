/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file detectr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Baumann
 * @date 11/11/2024
 */

#pragma once

#include <atomic>
#include <ovni.h>
#include "base_instr.hpp"


/**
 * Marker colors values of the default Paraver color palette
 */
enum mark_color : int64_t {
    MARK_COLOR_BLUE = 1,
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

    // debug printing method. Can be enable with the ENABLE_DEBUG flag included.
    #ifdef ENABLE_DEBUG
        #define debug_print(fmt, ...) \
            printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
    #else
        #define debug_print(fmt, ...)
    #endif

    // atomic counter of how many tasks got created
    std::atomic<int> ntasks_counter(0);

    // this boolean is needed if something other than DetectR has to be called.
    #define INSTRUMENTATION_ACTIVE true    

    #define INSTRUMENTATION_START()         \
        debug_print("instr_start (TID: %d)", get_tid());       \
        instrumentation_init_proc(0, 1);    \
        ovni_thread_require("taskr", "1.0.0")
    
    #define INSTRUMENTATION_END()                                               \
        debug_print("instr_end (TID: %d)", get_tid());                          \
        ovni_attr_set_double("taskr.ntasks", (double) ntasks_counter.load());   \
        instrumentation_end()

    #define INSTRUMENTATION_THREAD_INIT()                                           \
        debug_print("instr_thread_init with isready: %d (TID: %d)", ovni_thread_isready(), get_tid()); \
        if(!ovni_thread_isready()) {                                                \
            instrumentation_init_thread();                                          \
        }

    #define INSTRUMENTATION_THREAD_END()                                            \
        debug_print("instr_thread_end with isready: %d (TID: %d)", ovni_thread_isready(), get_tid());  \
        if(ovni_thread_isready()) {                                                 \
            instrumentation_thread_end();                                           \
            ovni_thread_free();                                                     \
        }

    #define INSTRUMENTATION_TASK_INIT(taskid)           \
        debug_print("instr_task_init: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_init(taskid);                  \
        ntasks_counter++
    
    #define INSTRUMENTATION_TASK_EXEC(taskid)           \
        debug_print("instr_task_exec: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_execute(taskid)

    #define INSTRUMENTATION_TASK_END(taskid)            \
        debug_print("instr_task_end: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_end(taskid)

    #define INSTRUMENTATION_TASK_SUSPEND(taskid)            \
        debug_print("instr_task_suspend: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_suspend(taskid)

    #define INSTRUMENTATION_TASK_FINISH(taskid)            \
        debug_print("instr_task_finish: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_finish(taskid)

    #define INSTRUMENTATION_TASK_NOTADD(taskid)            \
        debug_print("instr_task_notadd: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_notadd(taskid)

    #define INSTRUMENTATION_TASK_ADD(taskid)            \
        debug_print("instr_task_add: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_add(taskid)

    #define INSTRUMENTATION_TASK_READY(taskid)            \
        debug_print("instr_task_ready: %d (TID: %d)", (int) taskid, get_tid());   \
        instr_taskr_task_ready(taskid)    

    // markers
    #define INSTRUMENTATION_MARKER_INIT(flag)  \
        debug_print("instr_marker_init (TID: %d)", get_tid());   \
        marker_init(flag)

    #define INSTRUMENTATION_MARKER_ADD(str, value)  \
        debug_print("instr_marker_add (TID: %d)", get_tid());   \
        marker_add(str, value)

    #define INSTRUMENTATION_MARKER_SET(str)  \
        debug_print("instr_marker_set (TID: %d)", get_tid());   \
        marker_set(str)

    #define INSTRUMENTATION_MARKER_PUSH(str)  \
        debug_print("instr_marker_push (TID: %d)", get_tid());   \
        marker_push(str)

    #define INSTRUMENTATION_MARKER_POP(str)  \
        debug_print("instr_marker_pop (TID: %d)", get_tid());   \
        marker_pop(str)


#else   /* No instrumentation (void) */

    #define INSTRUMENTATION_ACTIVE false

    #define INSTRUMENTATION_START()

    #define INSTRUMENTATION_END()

    #define INSTRUMENTATION_THREAD_INIT()

    #define INSTRUMENTATION_THREAD_END()

    #define INSTRUMENTATION_TASK_INIT(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_EXEC(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_END(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_SUSPEND(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_FINISH(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_NOTADD(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_ADD(taskid) (void)(taskid)

    #define INSTRUMENTATION_TASK_READY(taskid) (void)(taskid)

    // markers
    #define INSTRUMENTATION_MARKER_INIT(flag) (void)(flag)

    #define INSTRUMENTATION_MARKER_ADD(str, value)  (void)(str); (void)(value)

    #define INSTRUMENTATION_MARKER_SET(str)  (void)(str)

    #define INSTRUMENTATION_MARKER_PUSH(str)  (void)(str)

    #define INSTRUMENTATION_MARKER_POP(str)  (void)(str)


#endif  /* USE_INSTRUMENTATION */