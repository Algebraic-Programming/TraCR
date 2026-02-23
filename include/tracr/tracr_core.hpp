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

namespace TraCR {

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
 * A way to check how many TraCR threads exists
 */
inline std::atomic<bool> tracr_proc_init{false};

/**
 * A way to check how many TraCR threads exists
 */
inline std::atomic<int> num_tracr_threads{0};

/**
 * Lazy add color definition method for the Paraver format
 */
inline std::atomic<uint16_t> lazy_colorId{23};

/**
 * A global boolean flag to check if we want to flush to file.
 */
inline std::atomic<bool> flush_enabled{true};

/**
 * User defined output path (default is current directory)
 */
inline std::string trace_folder_path{""};

/**
 *
 */
static inline void instrumentation_thread_init() {
  // Check if this C++ thread already has an Instance if so, abort
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

  // Increase global thread counter
  ++num_tracr_threads;

  // Add the new TraCR Thread
  tracrProc->addTraCRThread(this_tid);
}

/**
 *
 */
static inline void instrumentation_thread_finalize() {
  // Check if the tracr thread exists
  if (!tracrThread) {
    std::cerr << "TraCR Thread doesn't exist\n";
    std::exit(EXIT_FAILURE);
  }

  // If it exists check if it is inside the tracr proc list
  // If yes, erase it, else abort
  tracrProc->eraseTraCRThread(syscall(SYS_gettid));

  // Flushing the trace of this TraCR thread now
  if (flush_enabled) {
    tracrThread->flush_traces(tracrProc->getFolderPath());
  }

  // Finalize the thread now (destructor of it is also called)
  tracrThread.reset();

  // Decrease global thread counter
  --num_tracr_threads;
}

/**
 *
 */
static inline void instrumentation_start() {
  // Checking if the tracr Proc is ready
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
  if (flush_enabled) {
    if (!tracrProc->create_folder_recursive(trace_folder_path)) {
      std::cerr << "Folder creation did not work: " << trace_folder_path
                << "\n";
      std::exit(EXIT_FAILURE);
    }
  }

  // Initialize the tracr thread of this tracr proc
  instrumentation_thread_init();

  // TraCR Proc is now ready
  tracr_proc_init = true;
}

/**
 *
 */
static inline void instrumentation_end() {
  if (!tracrProc) {
    std::cerr << "TraCR Proc has not been initialized\n";
    std::exit(EXIT_FAILURE);
  }

  if (num_tracr_threads.load() == 0) {
    std::cerr << "No TraCR Thread existing counter: "
              << num_tracr_threads.load() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (num_tracr_threads.load() > 1) {
    std::cerr << "There are still some TraCR Threads running: "
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

  // Flushing the trace of this TraCR thread now (if enabled)
  if (flush_enabled) {
    tracrThread->flush_traces(tracrProc->getFolderPath());
  }

  // Destroys the TraCR Thread pointer and calls the destructor
  tracrThread.reset();

  // Decrease global thread counter
  --num_tracr_threads;

  // Dump TraCR Proc JSON file (if enabled)
  if (flush_enabled) {
    tracrProc->dump_JSON();
  }

  // Destroys the TraCR Proc pointer and calls the destructor
  tracrProc.reset();

  // TraCR Proc is now finalized
  tracr_proc_init = false;
}

/**
 * Debugging method for checking if something has been stored in tracr thread
 */
static inline std::string instrumentation_get_thread_trace_str() {
  // Safety checks
  if (!tracrThread) {
    return "[ERROR: No thread context]";
  }

  std::string tid_str =
      "Thread(" + std::to_string(tracrThread->getTID()) + "):";

  if (tracrThread->_traceIdx == 0) {
    return tid_str + "[EMPTY: No trace data]";
  }

  // Calculate total bytes
  size_t total_bytes = sizeof(Payload) * tracrThread->_traceIdx;
  const uint8_t *raw_data =
      reinterpret_cast<const uint8_t *>(tracrThread->_traces.data());

  // Convert to hex string
  std::stringstream hex_stream;
  hex_stream << std::hex << std::setfill('0');

  hex_stream << tid_str;

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
 * \param[in] label
 * \param[in] colorId
 *
 * @return the eventId of this marker
 */
static inline uint16_t
instrumentation_mark_w_color_add(const std::string &label,
                                 const uint16_t &colorId) {
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
static inline uint16_t instrumentation_mark_add(const std::string &label) {
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

  Payload payload{channelId, UINT16_MAX, UINT32_MAX, NanoTimer::now()};

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
static inline void instrumentation_enable_flush(const bool &enable_flush) {
  flush_enabled = enable_flush;
}

/**
 *
 */
static inline void instrumentation_trace_path(const std::string &path) {
  trace_folder_path = path;
}

/**
 *
 */
static inline bool instrumentation_is_proc_ready() {
  return tracr_proc_init.load();
}

/**
 *
 */
static inline int instrumentation_num_tracr_threads() {
  return num_tracr_threads.load();
}

/**
 *
 */
static inline bool instrumentation_proc_exists() {
  return (tracrProc != nullptr);
}

/**
 *
 */
static inline bool instrumentation_thread_exists() {
  return (tracrThread != nullptr);
}

/**
 *
 */
static inline std::string instrumentation_get_json_str() {
  tracrProc->write_JSON();
  return (tracrProc->_json_file).dump();
}

} // namespace TraCR