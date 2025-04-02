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
 * Using this flag will enable all the instrumentations of TraCR. Otherwise pure
 * void functions.
 */
#ifdef ENABLE_INSTRUMENTATION

/**
 * Keep track of the main thread as this one has to be free'd when instr_end is
 * called
 */
pid_t main_TID;

/**
 * A flag to check if something else has initialized ovni (like nOS-V). If so,
 * TraCR with not init/end proc.
 */
bool external_init;

/**
 * ovni thread marker methods
 */
ThreadMarkerMap thread_marker_map;
#endif
