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
#include <cstdlib>       // std::getenv
#include <sys/sysinfo.h> // get_nprocs()

#include "base_instr.hpp"
#include <ovni.h>

/**
 * Marker colors values of the default Paraver color palette
 * NOTE: normally Black would be illegal in ovni (But not in our modified
 * version of ovni ;P)!
 * Black means no color
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
 * Using this flag will enable all the instrumentations of TraCR. Otherwise pure
 * void functions.
 */
#if defined(INSTRUMENTATION_TASKS) || defined(INSTRUMENTATION_THREADS)

/**
 * Debug printing method. Can be enabled with the ENABLE_DEBUG flag included.
 */
#ifdef ENABLE_DEBUG
#define debug_print(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define debug_print(fmt, ...)
#endif

/**
 * Atomic counter of how many tasks got created
 */
inline std::atomic<int> ntasks_counter{0};

/**
 * Keep track of the main thread as this one has to be free'd when instr_end is
 * called
 */
inline pid_t main_TID;

/**
 * A flag to check if something else has initialized ovni (like nOS-V). If so,
 * TraCR with not init/end proc.
 */
inline bool external_init = false;

/**
 * An env variable disable the traces
 */
inline bool disable_tracr = false;

/**
 * automatic label ID assigning, i.e. doesn't matter which color the label is
 * assigned to
 */
inline std::atomic<int64_t> auto_label{23L};

/**
 * A function to check if the env flag is active or not
 */
inline bool get_env_flag() {
  const char *val = std::getenv("DISABLE_TRACR");
  if (!val)
    return false; // not set -> treat as false

  std::string s(val);
  // accept "1", "true", "TRUE", etc.
  return (s == "1" || s == "true" || s == "TRUE" || s == "on" || s == "ON");
}

/**
 * This boolean is needed if something other than TraCR has to be called.
 */
#define INSTRUMENTATION_ACTIVE true

/**
 * ovni proc methods
 */
#define INSTRUMENTATION_START()                                                \
  disable_tracr = get_env_flag();                                              \
  if (!disable_tracr) {                                                        \
    main_TID = get_tid();                                                      \
    external_init = ovni_proc_isready();                                       \
    debug_print("instr_start (external_init: %d) (TID: %d)", external_init,    \
                get_tid());                                                    \
    if (!external_init) {                                                      \
      instrumentation_init_proc(sched_getcpu(), get_nprocs());                 \
    }                                                                          \
    ovni_thread_require("taskr", "1.0.0");                                     \
  }

#define INSTRUMENTATION_END()                                                  \
  if (!disable_tracr) {                                                        \
    debug_print("instr_end (external_init: %d) (TID: %d)", external_init,      \
                get_tid());                                                    \
    ovni_attr_set_double("taskr.ntasks", (double)ntasks_counter.load());       \
    if (!external_init) {                                                      \
      instrumentation_end();                                                   \
    }                                                                          \
  }

/**
 * ovni thread methods
 */
#define INSTRUMENTATION_THREAD_INIT()                                          \
  if (!disable_tracr) {                                                        \
    debug_print("instr_thread_init with isready: %d (TID: %d)",                \
                ovni_thread_isready(), get_tid());                             \
    if (!ovni_thread_isready()) {                                              \
      instrumentation_init_thread();                                           \
    }                                                                          \
  }

#define INSTRUMENTATION_THREAD_END()                                           \
  if (!disable_tracr) {                                                        \
    debug_print(                                                               \
        "external_init %d, instr_thread_end with isready: %d (TID: %d)",       \
        external_init, ovni_thread_isready(), get_tid());                      \
    if (!external_init && ovni_thread_isready() && !(main_TID == get_tid())) { \
      instrumentation_thread_end();                                            \
      ovni_thread_free();                                                      \
    }                                                                          \
  }
#else

#define INSTRUMENTATION_ACTIVE false

/**
 * ovni proc methods
 */
#define INSTRUMENTATION_START()

#define INSTRUMENTATION_END()

/**
 * ovni thread methods
 */
#define INSTRUMENTATION_THREAD_INIT()

#define INSTRUMENTATION_THREAD_END()

#endif /* defined(INSTRUMENTATION_TASKS) || defined(INSTRUMENTATION_THREADS)   \
        */

/**
 *  task marker methods
 */
#ifdef INSTRUMENTATION_TASKS

inline TaskMarkerMap task_marker_map;

#define INSTRUMENTATION_TASK_INIT()                                            \
  if (!disable_tracr) {                                                        \
    debug_print("instr_task_init: (TID: %d)", get_tid());                      \
    ntasks_counter++;                                                          \
  }

#define INSTRUMENTATION_TMARK_INIT(chan_type)                                  \
  if (!disable_tracr) {                                                        \
    debug_print("instr_task_type_set: (TID: %d)", get_tid());                  \
    ovni_attr_set_double("taskr.chan_type", (double)chan_type);                \
  }

#define INSTRUMENTATION_TMARK_ADD(labelid, label)                              \
  (!disable_tracr ? task_marker_map.add(labelid, label) : 0)

#define INSTRUMENTATION_TMARK_LAZY_ADD(label)                                  \
  (!disable_tracr ? task_marker_map.add(auto_label++, label) : 0)

#define INSTRUMENTATION_TMARK_SET(taskid, idx)                                 \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int)taskid,         \
                (int)idx, get_tid());                                          \
    task_marker_map.set(taskid, idx);                                          \
  }

#define INSTRUMENTATION_TMARK_PUSH(taskid, idx)                                \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int)taskid,         \
                (int)idx, get_tid());                                          \
    task_marker_map.push(taskid, idx);                                         \
  }

#define INSTRUMENTATION_TMARK_POP(taskid, idx)                                 \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_mark_set: %d, %d (TID: %d)", (int)taskid,         \
                (int)idx, get_tid());                                          \
    task_marker_map.pop(taskid, idx);                                          \
  }

// Resets the color to none
#define INSTRUMENTATION_TMARK_RESET(taskid)                                    \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_reset (TID: %d)", get_tid());                    \
    instr_taskr_mark_set(taskid, MARK_COLOR_BLACK);                            \
  }

// A method for storing the user-defined labels of the taskid
// Please provide it as a JSON file in string format
#define INSTRUMENTATION_TMARK_NAMES(task_map_str)                              \
  if (!disable_tracr) {                                                        \
    debug_print("instr_tmark_names (TID: %d)", get_tid());                     \
    ovni_attr_set_json("taskr.tmark_names", task_map_str);                     \
  }

/**
 * vanilla task markers (VTMARK)
 */

#define INSTRUMENTATION_VTMARK_INIT(chan_type)                                 \
  INSTRUMENTATION_TMARK_INIT(chan_type)

#define INSTRUMENTATION_VTMARK_ADD(color, label)                               \
  if (!disable_tracr) {                                                        \
    debug_print(                                                               \
        "ovni_taskr_vmark_add (TID: %d) with color: %ld and label: %s",        \
        get_tid(), color, label);                                              \
    instr_taskr_mark_create(color, label);                                     \
  }

#define INSTRUMENTATION_VTMARK_SET(taskid, color)                              \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_vmark_set: taskid: %d, color: %ld (TID: %d)",     \
                taskid, color, get_tid());                                     \
    instr_taskr_mark_set(taskid, color);                                       \
  }

#define INSTRUMENTATION_VTMARK_PUSH(taskid, color)                             \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_vmark_push: taskid: %d, color: %ld (TID: %d)",    \
                taskid, color, get_tid());                                     \
    instr_taskr_mark_push(taskid, color);                                      \
  }

#define INSTRUMENTATION_VTMARK_POP(taskid, color)                              \
  if (!disable_tracr) {                                                        \
    debug_print("instr_taskr_vmark_pop: taskid: %d, color: %ld (TID: %d)",     \
                taskid, color, get_tid());                                     \
    instr_taskr_mark_pop(taskid, color);                                       \
  }

#else /* No task instrumentations (void) */

#define INSTRUMENTATION_TASK_INIT()

#define INSTRUMENTATION_TMARK_INIT(chan_type) (void)(chan_type)

#define INSTRUMENTATION_TMARK_ADD(labelid, label)                              \
  -1;                                                                          \
  (void)(labelid);                                                             \
  (void)(label)

#define INSTRUMENTATION_TMARK_LAZY_ADD(label)                                  \
  -1;                                                                          \
  (void)(label)

#define INSTRUMENTATION_TMARK_SET(taskid, idx)                                 \
  (void)(taskid);                                                              \
  (void)(idx)

#define INSTRUMENTATION_TMARK_PUSH(taskid, idx)                                \
  (void)(taskid);                                                              \
  (void)(idx)

#define INSTRUMENTATION_TMARK_POP(taskid, idx)                                 \
  (void)(taskid);                                                              \
  (void)(idx)

#define INSTRUMENTATION_TMARK_RESET(taskid) (void)(taskid)

#define INSTRUMENTATION_TMARK_NAMES(task_map_str) (void)(task_map_str)

/**
 * vanilla task markers (VTMARK)
 */
#define INSTRUMENTATION_VTMARK_INIT(chan_type) (void)(chan_type)

#define INSTRUMENTATION_VTMARK_ADD(color, label)                               \
  (void)(color);                                                               \
  (void)(label)

#define INSTRUMENTATION_VTMARK_SET(taskid, color)                              \
  (void)(taskid);                                                              \
  (void)(color)

#define INSTRUMENTATION_VTMARK_PUSH(taskid, color)                             \
  (void)(taskid);                                                              \
  (void)(color)

#define INSTRUMENTATION_VTMARK_POP(taskid, color)                              \
  (void)(taskid);                                                              \
  (void)(color)

#endif /* INSTRUMENTATION_TASKS */

/**
 * ovni thread marker methods
 */
#ifdef INSTRUMENTATION_THREADS

inline ThreadMarkerMap thread_marker_map;

#define INSTRUMENTATION_MARK_INIT(flag)                                        \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_init (TID: %d)", get_tid());                     \
    ovni_mark_type(0, flag, "TraCR Thread Markers");                           \
  }

#define INSTRUMENTATION_MARK_ADD(labelid, label)                               \
  (!disable_tracr ? thread_marker_map.add(labelid, label) : 0)

#define INSTRUMENTATION_MARK_LAZY_ADD(label)                                   \
  (!disable_tracr ? thread_marker_map.add(auto_label++, label) : 0)

#define INSTRUMENTATION_MARK_SET(idx)                                          \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_set idx: %ld (TID: %d)", idx, get_tid());        \
    thread_marker_map.set(idx);                                                \
  }

#define INSTRUMENTATION_MARK_PUSH(idx)                                         \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_push (TID: %d)", get_tid());                     \
    thread_marker_map.push(idx);                                               \
  }

#define INSTRUMENTATION_MARK_POP(idx)                                          \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_pop (TID: %d)", get_tid());                      \
    thread_marker_map.pop(idx);                                                \
  }

// Resets the color to none
#define INSTRUMENTATION_MARK_RESET()                                           \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_reset (TID: %d)", get_tid());                    \
    ovni_mark_set(0, INT64_MAX);                                               \
  }

/**
 * ovni marker methods (vanilla)
 */

// Same as INSTRUMENTATION_MARK_INIT but with a different title
#define INSTRUMENTATION_VMARK_INIT(flag)                                       \
  if (!disable_tracr) {                                                        \
    debug_print("instr_vmarker_init (TID: %d)", get_tid());                    \
    ovni_mark_type(0, flag, "TraCR Thread VMarkers");                          \
  }

#define INSTRUMENTATION_VMARK_ADD(color, label)                                \
  if (!disable_tracr) {                                                        \
    debug_print("ovni_mark_label (TID: %d) with color: %ld and label: %s",     \
                get_tid(), color, label);                                      \
    ovni_mark_label(0, color, label);                                          \
  }

#define INSTRUMENTATION_VMARK_SET(color)                                       \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_set (TID: %d) with color %ld", get_tid(),        \
                color);                                                        \
    ovni_mark_set(0, color);                                                   \
  }

#define INSTRUMENTATION_VMARK_PUSH(color)                                      \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_push (TID: %d) with color %ld", get_tid(),       \
                color);                                                        \
    ovni_mark_push(0, color);                                                  \
  }

#define INSTRUMENTATION_VMARK_POP(color)                                       \
  if (!disable_tracr) {                                                        \
    debug_print("instr_marker_pop (TID: %d) with color %ld", get_tid(),        \
                color);                                                        \
    ovni_mark_pop(0, color);                                                   \
  }

#else /* No thread instrumentations (void) */

/**
 * ovni thread marker methods
 */
#define INSTRUMENTATION_MARK_INIT(flag) (void)(flag)

#define INSTRUMENTATION_MARK_ADD(labelid, label)                               \
  -1;                                                                          \
  (void)(labelid);                                                             \
  (void)(label)

#define INSTRUMENTATION_MARK_LAZY_ADD(label)                                   \
  -1;                                                                          \
  (void)(label)

#define INSTRUMENTATION_MARK_SET(idx) (void)(idx)

#define INSTRUMENTATION_MARK_PUSH(idx) (void)(idx)

#define INSTRUMENTATION_MARK_POP(idx) (void)(idx)

#define INSTRUMENTATION_MARK_RESET()

/**
 * ovni marker methods (vanilla)
 */
#define INSTRUMENTATION_VMARK_INIT(flag) (void)(flag)

#define INSTRUMENTATION_VMARK_ADD(color, label)                                \
  (void)(color);                                                               \
  (void)(label)

#define INSTRUMENTATION_VMARK_SET(color) (void)(color)

#define INSTRUMENTATION_VMARK_PUSH(color) (void)(color)

#define INSTRUMENTATION_VMARK_POP(color) (void)(color)

#endif /* INSTRUMENTATION_THREADS */
