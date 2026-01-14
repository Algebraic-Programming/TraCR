/*
 *   Copyright 2026 Huawei Technologies Co., Ltd.
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
 * @file tracr_core.hpp
 * @brief TraCR core functionalities
 * @author Noah Andrés Baumann
 * @date 08/01/2026
 */

#pragma once

#include <atomic>
#include <nlohmann/json.hpp>
#include <string>
#include <sys/syscall.h> // syscall()
#include <unistd.h>      // SYS_gettid
#include <unordered_map>

#include "marker_management_engine.hpp"

/**
 * Global TraCR proc place holder
 */
static inline std::unique_ptr<TraCRProc> tracrProc;

/**
 * Global TraCR thread place holder (on the level of threads)
 */
static inline thread_local std::unique_ptr<TraCRThread> tracrThread;

/**
 * Global variable to check if TraCR tracing is enabled (at runtime)
 */
static inline std::atomic<bool> enable_tracr{true};

/**
 *
 */
static inline void instrumentation_start(const std::string &path = "") {
  if (!enable_tracr) {
    return;
  }
  // This could be also checked with if(!tracrProc){} but would not be thread
  // safe
  if (tracr_proc_init.load()) {
    std::cerr << "TraCR Proc has already been initialized by the thread: "
              << tracrProc->getTID() << "\n";
    std::exit(EXIT_FAILURE);
  }

  // Get current thread ID
  pid_t tid = syscall(SYS_gettid);

  // Initialize the TraCRProc
  tracrProc = std::make_unique<TraCRProc>(tid);

  // Create the folders to store the traces
  if (!tracrProc->create_folder_recursive(path)) {
    std::cerr << "Folder creation did not work: " << path << "\n";
    std::exit(EXIT_FAILURE);
  }

  // Add tracr Thread
  tracrThread = std::make_unique<TraCRThread>(tid);

  // Add its TraCRThread TID
  tracrProc->addTraCRThread(tid);
}

/**
 *
 */
static inline void instrumentation_end() {
  if (!enable_tracr) {
    return;
  }

  if (!tracr_proc_init.load()) {
    std::cerr << "TraCR Proc has not been initialized by the thread: "
              << tracrProc->getTID() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (tracrProc->_tracrThreadIDs.size() != 1) {
    std::cerr << "TraCR Proc should only have his thread left but got: "
              << tracrProc->_tracrThreadIDs.size() << "\n";
    std::exit(EXIT_FAILURE);
  }

  // Get current thread ID
  pid_t tid = syscall(SYS_gettid);

  if (tracrProc->_tracrThreadIDs[0] != tid) {
    std::cerr << "TraCR instrumentation_end called by thread: " << tid
              << " instead of the main thread: "
              << tracrProc->_tracrThreadIDs[0] << "\n";
    std::exit(EXIT_FAILURE);
  }

  // Set the global boolean back to not being initialized
  tracr_proc_init = false;

  // Flushing the trace of this TraCR thread now
  tracrThread->flush_traces(tracrProc->getFolderPath());

  // Destroys the TraCR Thread pointer and calls the destructor
  tracrThread.reset();

  // Dump TraCR Proc JSON file
  tracrProc->dump_JSON();

  // Destroys the TraCR Proc pointer and calls the destructor
  tracrProc.reset();
}

/**
 *
 */
static inline void instrumentation_thread_init() {
  if (!enable_tracr) {
    return;
  }

  // Check
  if (tracrThread) {
    std::cerr << "TraCR Thread already exists with TID: "
              << tracrThread->getTID() << "\n";
    std::exit(EXIT_FAILURE);
  }

  pid_t this_tid = syscall(SYS_gettid);

  for (const auto &tid : tracrProc->_tracrThreadIDs) {
    if (this_tid == tid) {
      std::cerr << "TraCR thread with this TID already exists in the list in "
                   "tracr proc\n";
      std::exit(EXIT_FAILURE);
    }
  }

  // Add tracr Thread
  tracrThread = std::make_unique<TraCRThread>(this_tid);

  // Add the new TraCR Thread
  tracrProc->addTraCRThread(this_tid);
}

/**
 *
 */
static inline void instrumentation_thread_finalize() {
  if (!enable_tracr) {
    return;
  }

  // Check if the tracr thread exists
  if (!tracrThread) {
    std::cerr << "TraCR Thread doesn't exist\n";
    std::exit(EXIT_FAILURE);
  }

  // If it exists check if it is inside the tracr proc list
  pid_t this_tid = syscall(SYS_gettid);
  bool is_in_list = false;
  for (auto it = tracrProc->_tracrThreadIDs.begin();
       it != tracrProc->_tracrThreadIDs.end(); ++it) {

    if (this_tid == (*it)) {
      if (it == tracrProc->_tracrThreadIDs.begin()) {
        std::cerr
            << "It is NOT allowed to thread_finalize the TraCR Proc thread!\n";
        std::exit(EXIT_FAILURE);
      }

      is_in_list = true;
      tracrProc->_tracrThreadIDs.erase(it);
      break;
    }
  }

  if (!is_in_list) {
    std::cerr << "TraCR Thread is not inside the TraCR Proc list\n";
    std::exit(EXIT_FAILURE);
  }

  // Flushing the trace of this TraCR thread now
  tracrThread->flush_traces(tracrProc->getFolderPath());

  // Finalize the thread now (destructor of it is also called)
  tracrThread.reset();
}

/**
 * Marker add method
 *
 * NOTE: This is note thread safe! Should be called by one thread.
 *
 * \param[in] colorId
 * \param[in] label
 *
 * @return the eventId of this marker
 */
static inline uint16_t instrumentation_mark_add(const uint16_t &colorId,
                                                const std::string &label) {
  if (!enable_tracr) {
    return 0;
  }

  if (tracrProc->_markerTypes.count(colorId)) {
    std::cerr << "This color has already been used. Choose another one.\n";
    std::exit(EXIT_FAILURE);
  }

  tracrProc->_markerTypes[colorId] = label;

  return tracrProc->_markerTypes.size() - 1;
}

/**
 * Lazy marker add method. I.e. one doesn't have to provide the color idx
 *
 * NOTE: This is note thread safe! Should be called by one thread.
 *
 * \param[in] label
 *
 * @return the eventId of this marker
 */
static inline uint16_t instrumentation_mark_lazy_add(const std::string &label) {
  if (!enable_tracr) {
    return 0;
  }

  uint16_t colorId = lazy_colorId.fetch_add(1);
  if (tracrProc->_markerTypes.count(colorId)) {
    std::cerr << "This color has already been used. Choose another one.\n";
    std::exit(EXIT_FAILURE);
  }

  tracrProc->_markerTypes[colorId] = label;

  return tracrProc->_markerTypes.size() - 1;
}

/**
 *
 */
static inline void
instrumentation_mark_set(const uint16_t &channelId, const uint16_t &eventId,
                         const uint32_t &extraId = UINT32_MAX) {
  if (!enable_tracr) {
    return;
  }

  Payload payload{channelId, eventId, extraId, NanoTimer::now()};

  tracrThread->store_trace(payload);
}

/**
 *
 */
static inline void instrumentation_mark_reset(const uint16_t &channelId) {
  if (!enable_tracr) {
    return;
  }

  Payload payload{channelId, UINT16_MAX, 0, NanoTimer::now()};

  tracrThread->store_trace(payload);
}

/**
 *
 */
static inline void instrumentation_on() { enable_tracr = true; }

/**
 *
 */
static inline void instrumentation_off() { enable_tracr = false; }