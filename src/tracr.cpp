/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file tracr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Baumann
 * @date 17/12/2024
 */

#include "tracr.hpp"

/**
 * 
 */
#if defined(INSTRUMENTATION_TASKS) || defined(INSTRUMENTATION_THREADS)
    // atomic counter of how many tasks got created
    std::atomic<int> ntasks_counter(0);

    // keep track of the main thread as this one has to be free'd when instr_end is called
    pid_t main_TID;
#endif

/**
 * 
 */
#ifdef INSTRUMENTATION_TASKS
    TaskMarkerMap task_marker_map;
#endif

/**
 * 
 */
#ifdef INSTRUMENTATION_THREADS
    ThreadMarkerMap thread_marker_map;
#endif