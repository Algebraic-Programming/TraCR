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

#include <tracr/tracr.hpp>

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
 * An env variable to disable the traces
 */
bool disable_tracr = false;

/**
 * automatic label ID assigning, i.e. doesn't matter which color the label is
 * assigned to
 */
std::atomic<int64_t> auto_label{23L};

/**
 * A function to check if the env flag DISABLE_TRACR is active or not
 */
bool get_env_flag() {
  const char *val = std::getenv("DISABLE_TRACR");
  if (!val)
    return false; // not set -> treat as false

  std::string s(val);
  // accept "1", "true", "TRUE", etc.
  return (s == "1" || s == "true" || s == "TRUE" || s == "on" || s == "ON");
}

/**
 * ovni thread marker methods
 */
ThreadMarkerMap thread_marker_map;

#endif /* ENABLE_INSTRUMENTATION */
