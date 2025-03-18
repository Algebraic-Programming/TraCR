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
#ifdef ENABLE_INSTRUMENTATION
    // keep track of the main thread as this one has to be free'd when instr_end is called
    pid_t main_TID;

    // A flag to check if something else has initialized ovni (like nosv). If so, TraCR with not init/end proc.
    bool external_init;

    // thread marker's mapping
    ThreadMarkerMap thread_marker_map;
#endif
