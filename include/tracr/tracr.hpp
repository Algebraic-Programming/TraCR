/*
 *   Copyright 2025 Huawei Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file tracr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Baumann
 * @date 17/12/2024
 */

#pragma once

#include <atomic>
#include <sys/sysinfo.h> // get_nprocs()
#include <cstdlib>       // std::getenv

#include <ovni.h>
#include "base_instr.hpp"

/**
 * Marker colors values of the default Paraver color palette
 * NOTE: Black would be 0 but in ovni it is illegal!
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
 * Using this flag will enable all the instrumentations of TraCR. Otherwise pure
 * void functions.
 */
#ifdef ENABLE_INSTRUMENTATION

/**
 * Debug printing method. Can be enabled with the ENABLE_DEBUG flag included.
 */
#ifdef ENABLE_DEBUG
#define debug_print(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define debug_print(fmt, ...)
#endif

/**
 * Keep track of the main thread as this one has to be free'd when instr_end is
 * called
 */
extern pid_t main_TID;

/**
 * A flag to check if something else has initialized ovni (like nOS-V). If so,
 * TraCR with not init/end proc.
 */
extern bool external_init;

/**
 * An env variable disable the traces
 */
extern bool disable_tracr;

/**
 * automatic label ID assigning, i.e. doesn't matter which color the label is assigned to
 */
extern std::atomic<int64_t> auto_label;

/**
 * A function to check if the env flag is active or not
 */
extern bool get_env_flag();

/**
 * This boolean is needed if something other than TraCR has to be called.
 */
#define INSTRUMENTATION_ACTIVE true

/**
 * ovni proc methods
 */
#define INSTRUMENTATION_START(external_init_)                                  \
  disable_tracr = get_env_flag();                                              \
  if(!disable_tracr) {                                                         \
    main_TID = get_tid();                                                      \
    external_init = external_init_;                                            \
    debug_print("instr_start (external_init: %d) (TID: %d)", external_init,    \
                get_tid());                                                    \
    if (!external_init) {                                                      \
      instrumentation_init_proc(sched_getcpu(), get_nprocs());                 \
    }                                                                          \
  }

#define INSTRUMENTATION_END()                                                  \
  if(!disable_tracr) {                                                         \
    debug_print("instr_end (external_init: %d) (TID: %d)", external_init,      \
                get_tid());                                                    \
    if (!external_init) {                                                      \
      instrumentation_end();                                                   \
    }                                                                          \
  }

/**
 * ovni thread methods
 */
#define INSTRUMENTATION_THREAD_INIT()                                          \
  if(!disable_tracr) {                                                         \
    debug_print("instr_thread_init with isready: %d (TID: %d)",                \
                ovni_thread_isready(), get_tid());                             \
    if (!ovni_thread_isready()) {                                              \
      instrumentation_init_thread();                                           \
    }                                                                          \
  }

#define INSTRUMENTATION_THREAD_END()                                             \
  if(!disable_tracr) {                                                           \
    debug_print("external_init %d, instr_thread_end with isready: %d (TID: %d)", \
                external_init, ovni_thread_isready(), get_tid());                \
    if (!external_init && ovni_thread_isready() && !(main_TID == get_tid())) {   \
      instrumentation_thread_end();                                              \
      ovni_thread_free();                                                        \
    }                                                                            \
  }

/**
 * ovni thread marker methods
 */
extern ThreadMarkerMap thread_marker_map;

#define INSTRUMENTATION_MARK_INIT(flag)                                 \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_init (TID: %d)", get_tid());              \
    ovni_mark_type(0, flag, "TraCR Thread Markers");                    \
  }

#define INSTRUMENTATION_MARK_ADD(labelid, label)                        \
  (!disable_tracr ?                                                     \
    thread_marker_map.add(labelid, label): 0)                           \

#define INSTRUMENTATION_MARK_LAZY_ADD(label)                            \
  (!disable_tracr ?                                                     \
    thread_marker_map.add(auto_label++, label) : 0)                     \

#define INSTRUMENTATION_MARK_SET(idx)                                   \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_set idx: %ld (TID: %d)", idx, get_tid()); \
    thread_marker_map.set(idx);                                         \
  }

#define INSTRUMENTATION_MARK_PUSH(idx)                                  \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_push (TID: %d)", get_tid());              \
    thread_marker_map.push(idx);                                        \
  }

#define INSTRUMENTATION_MARK_POP(idx)                                   \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_pop (TID: %d)", get_tid());               \
    thread_marker_map.pop(idx);                                         \
  }

// Resets the color to none
#define INSTRUMENTATION_MARK_RESET()                                    \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_reset (TID: %d)", get_tid());             \
    ovni_mark_set(0, INT64_MAX);                                        \
  }

/**
 * ovni marker methods (vanilla)
 */
#define INSTRUMENTATION_VMARK_TYPE(flag, title)                         \
  if(!disable_tracr) {                                                  \
    ovni_mark_type(0, flag, title);                                     \
  }

#define INSTRUMENTATION_VMARK_LABEL(value, label)                       \
  if(!disable_tracr) {                                                  \
    ovni_mark_label(0, value, label);                                   \
  }

#define INSTRUMENTATION_VMARK_SET(value)                                \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_set (TID: %d)", get_tid());               \
    ovni_mark_set(0, value);                                            \
  }

#define INSTRUMENTATION_VMARK_PUSH(value)                               \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_push (TID: %d)", get_tid());              \
    ovni_mark_push(0, value);                                           \
  }

#define INSTRUMENTATION_VMARK_POP(value)                                \
  if(!disable_tracr) {                                                  \
    debug_print("instr_marker_pop (TID: %d)", get_tid());               \
    ovni_mark_pop(0, value);                                            \
  }

#else /* No instrumentations (void) */

#define INSTRUMENTATION_ACTIVE false

/**
 * ovni proc methods
 */
#define INSTRUMENTATION_START(external_init_) (void)(external_init_)

#define INSTRUMENTATION_END()

/**
 * ovni thread methods
 */
#define INSTRUMENTATION_THREAD_INIT()

#define INSTRUMENTATION_THREAD_END()

/**
 * ovni thread marker methods
 */
#define INSTRUMENTATION_MARK_INIT(flag) (void)(flag)

#define INSTRUMENTATION_MARK_ADD(labelid, label)                      \
  -1;                                                                 \
  (void)(labelid);                                                    \
  (void)(label)

#define INSTRUMENTATION_MARK_LAZY_ADD(label)                          \
  -1;                                                                 \
  (void)(label)

#define INSTRUMENTATION_MARK_SET(idx) (void)(idx)

#define INSTRUMENTATION_MARK_PUSH(idx) (void)(idx)

#define INSTRUMENTATION_MARK_POP(idx) (void)(idx)

#define INSTRUMENTATION_MARK_RESET()

/**
 * ovni marker methods (vanilla)
 */
#define INSTRUMENTATION_VMARK_TYPE(flag, title)                         \
  (void)(flag);                                                         \
  (void)(title)

#define INSTRUMENTATION_VMARK_LABEL(value, label)                       \
  (void)(value);                                                        \
  (void)(label)

#define INSTRUMENTATION_VMARK_SET(value) (void)(value)

#define INSTRUMENTATION_VMARK_PUSH(value) (void)(value)

#define INSTRUMENTATION_VMARK_POP(value) (void)(value)


#endif /* ENABLE_INSTRUMENTATION */
