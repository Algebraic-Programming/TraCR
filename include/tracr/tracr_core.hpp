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
 * A method to only let ONE thread to call tracr proc init
 */
static inline std::once_flag tracr_proc_once;

/**
 * A way to check if the TraCRProc has been initialized, if not, throw at
 * runtime.
 */
inline std::atomic<int> num_tracr_threads{0};

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
static inline void instrumentation_proc_init(const std::string &path = "") {
  // Check if the proc has already been initialized
  if (num_tracr_threads.load() > 0) {
    std::cerr << "TraCR Proc has already been initialized as there are threads: "
              << num_tracr_threads.load() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (tracrProc) {
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

  // Increase global number of tracr thread counter
  ++num_tracr_threads;
}

/**
 *
 */
static inline void instrumentation_proc_finalize() {

  debug_print("instrumentation_proc_finalize");

  if (num_tracr_threads.load() == 0) {
    std::cerr << "TraCR Proc has not been initialized\n";
    std::exit(EXIT_FAILURE);
  }

  if (num_tracr_threads.load() > 1) {
    std::cerr << "Can't finalize TraCR Proc as there are still running TraCR threads: "
              << num_tracr_threads.load() << "\n";
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

  // Check if thread is already initialized
  if (!tracrThread) {
    std::cerr << "TraCR Thread already finalized?\n";
    std::exit(EXIT_FAILURE);
  }

  // Flushing the trace of this TraCR thread now
  tracrThread->flush_traces(tracrProc->getFolderPath());

  // Destroys the TraCR Thread pointer and calls the destructor
  tracrThread.reset();

  // Dump TraCR Proc JSON file
  tracrProc->dump_JSON();
  
  // Destroys the TraCR Proc pointer and calls the destructor
  tracrProc.reset();
  
  // Decrease global number of tracr thread counter
  --num_tracr_threads;
}

/**
 *
 */
static inline void instrumentation_thread_init(const pid_t& main_thread_tid = -1) {

  // Check if thread is already initialized
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

  // Increase global number of tracr thread counter
  ++num_tracr_threads;
}

/**
 *
 */
static inline void instrumentation_thread_finalize() {
  debug_print("instrumentation_thread_finalize");

  // Check if the tracr thread exists
  if (!tracrThread) {
    std::cerr << "TraCR Thread doesn't exist\n";
    std::exit(EXIT_FAILURE);
  }

  // If it exists check if it is inside the tracr proc list
  // If yes, erase it, else abort
  tracrProc->eraseTraCRThread(syscall(SYS_gettid));

  // Flushing the trace of this TraCR thread now
  tracrThread->flush_traces(tracrProc->getFolderPath());

  // Finalize the thread now (destructor of it is also called)
  tracrThread.reset();

  // Decrease global number of tracr thread counter
  --num_tracr_threads;
}

/**
 * This is a thread-safe way to initialize the proc and there threads
 */
static inline void instrumentation_start(const std::string &path = "") {
  std::call_once(tracr_proc_once, [path]() {
    instrumentation_proc_init(path);
  });

  // Phase 2: initialize per-thread thread-local tracrThread
  pid_t tid = syscall(SYS_gettid);
  if (tid != tracrProc->getTID()) {  // skip master thread
    instrumentation_thread_init();
  }
}

/**
 *
 */
static inline void instrumentation_end() {
  // Check if proc still exists
  if (!tracrProc) {
    std::cerr << "TraCR Proc already finalized or has not been initialized?\n";
    std::exit(EXIT_FAILURE);
  }

  pid_t this_tid = syscall(SYS_gettid);

  if(this_tid == tracrProc->getTID()) {
    while (num_tracr_threads.load() > 1) {}
    instrumentation_proc_finalize();
  } else {
    instrumentation_thread_finalize();
  }

  if (num_tracr_threads.load() != 0) {
    std::cerr << "TraCR can't finalize as not all the threads have finished: remaining number of threads: %d\n";
    std::exit(EXIT_FAILURE);
  }
}

/**
 *
 */
static inline std::string instrumentation_get_thread_trace_str() {
  if (!enable_tracr) {
    return "";
  }

  // Safety checks
  if (!tracrThread) {
    return "[ERROR: No thread context]";
  }

  if (tracrThread->_traceIdx == 0) {
    return "[EMPTY: No trace data]";
  }

  // Calculate total bytes
  size_t total_bytes = sizeof(Payload) * tracrThread->_traceIdx;
  const uint8_t *raw_data =
      reinterpret_cast<const uint8_t *>(tracrThread->_traces.data());

  // Convert to hex string
  std::stringstream hex_stream;
  hex_stream << std::hex << std::setfill('0');

  for (size_t i = 0; i < total_bytes; ++i) {
    // Each byte as two hex digits
    hex_stream << std::setw(2) << static_cast<int>(raw_data[i]);

    // Add space every 4 bytes for readability
    if ((i + 1) % 4 == 0 && (i + 1) != total_bytes) {
      hex_stream << " ";
    }

    // New line every 16 bytes
    if ((i + 1) % 16 == 0 && (i + 1) != total_bytes) {
      hex_stream << "\n";
    }
  }

  return hex_stream.str();
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

/**
 *
 */
static inline bool instrumentation_is_proc_ready() {
  return (num_tracr_threads.load() == 1);
}

/**
 *
 */
static inline std::string instrumentation_get_json_str() {
  tracrProc->write_JSON();
  return (tracrProc->_json_file).dump();
}