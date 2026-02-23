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
 * @file marker_management_engine.hpp
 * @brief Marker collection and storing mechanism
 * @author Noah Andrés Baumann
 * @date 08/01/2026
 */

#pragma once

#include <array>
#include <atomic>
#include <ctime>
#include <fstream> // To store files
#include <iomanip>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sched.h> // sched_getcpu()
#include <string>
#include <sys/stat.h>  // mkdir()
#include <sys/types.h> // chmod type
#include <unistd.h>    // SYS_gettid
#include <unordered_map>

namespace TraCR {

/**
 * The maximum capacity of one tracr thread for capturing the traces.
 * Currently, we fix it here. Might be definable by the user.
 *
 * capatity = 2**16 = 65'536     -> ~1MB tracr thread size
 * capacity = 2**20 = 1'048'576  -> ~17MB tracr thread size (default)
 * capacity = 2**24 = 16'777'216 -> ~268MB tracr thread size
 */
#ifndef TRACR_CAPACITY
constexpr size_t CAPACITY = 1 << 20;
#else
constexpr size_t CAPACITY = TRACR_CAPACITY;
#endif

/**
 * Debug printing method. Can be enabled with the ENABLE_DEBUG flag included.
 */
#ifdef ENABLE_DEBUG
#define debug_print(fmt, ...) printf("[TraCR DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define debug_print(fmt, ...)
#endif

/**
 * Our nanosecond timer
 *
 * This timer can also be changed by the chrono (or PyPTO get_cycle()) method
 */
class NanoTimer {
public:
  // get current time in nanoseconds
  static uint64_t now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
           static_cast<uint64_t>(ts.tv_nsec);
  }
};

/**
 * Marker payload
 */
struct Payload {
  // channelId defines in which channel this payload has to be set [0, 65535]
  uint16_t channelId;

  // eventId defines the type of even (i.e. the color) [0, 65535]
  uint16_t eventId;

  // extraId consists of an extra information that has been added to be stored
  // as well (i.e. The type of task label of the event type)
  uint32_t extraId;

  // Chrono nanosecond timestamp
  uint64_t timestamp;
};

/**
 * TraCR Thread class. One MPI instance chas atleast 1
 */
class TraCRThread {
public:
  /**
   * Constructor
   */
  TraCRThread(const pid_t &tid) : _tid(tid){};

  /**
   * No default constructor allowed.
   */
  TraCRThread() = delete;

  /**
   * Default Destructor as we obey RAII
   */
  ~TraCRThread() = default;

  /**
   *
   */
  inline void store_trace(const Payload &payload) {
#ifdef TRACR_POLICY_PERIODIC
    if (_traceIdx == CAPACITY) {
      debug_print("WARNING: TID[%d] is full, this thread will now overwrite "
                  "from the beginning.",
                  _tid);
    }

    _traces[_traceIdx % CAPACITY] = payload;
    ++_traceIdx;

#elif defined(TRACR_POLICY_IGNORE_IF_FULL)
    if (_traceIdx >= CAPACITY) {
      debug_print("WARNING: TID[%d] is full, this thread will now ignore "
                  "incoming traces.",
                  _tid);
    } else {
      _traces[_traceIdx] = payload;
      ++_traceIdx;
    }
#else /* Abort if full */
    if (_traceIdx >= CAPACITY) {
      std::cerr << "Warning: TID[]" << _tid
                << " is full, terminating with a Runtime Error.\n";
      std::exit(EXIT_FAILURE);
    }

    _traces[_traceIdx] = payload;
    ++_traceIdx;
#endif
  }

  /**
   * Flushed the traces into a file at the given path
   */
  inline void flush_traces(const std::string &path) {
    // Don't create a folder if this TraCR thread is empty
    if (_traceIdx == 0) {
      return;
    }

    _thread_folder_name = path + "thread." + std::to_string(_tid) + "/";

    // Create the last thread ID folder
    if (mkdir(_thread_folder_name.c_str(), 0755) != 0) {
      if (errno != EEXIST) { // ignore "already exists"
        std::cerr << "mkdir failed for: " << _thread_folder_name
                  << " errno=" << errno << " (" << std::strerror(errno)
                  << ")\n";
        std::exit(EXIT_FAILURE);
      }
    }

    std::string filepath = _thread_folder_name + "traces.bts";

    debug_print("The filepath of this TraCR thread[%d] is: %s", _tid,
                filepath.c_str());

    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) {
      std::cerr << "Failed to open file: " << filepath << "\n";
      std::exit(EXIT_FAILURE);
    }

    // Write raw memory
    ofs.write(reinterpret_cast<const char *>(_traces.data()),
              sizeof(Payload) * _traceIdx);

    if (!ofs.good()) {
      std::cerr << "Failed to write into file: " << filepath << "\n";
      std::exit(EXIT_FAILURE);
    }

    // Closing file
    ofs.close();

    if (ofs.fail()) {
      std::cerr << "Failed to close file: " << filepath << "\n";
      std::exit(EXIT_FAILURE);
    }
  }

  /**
   *
   */
  inline pid_t getTID() { return _tid; }

  // The array to keep track of the traces
  std::array<Payload, CAPACITY> _traces;

  // The index at which point to add the next marker
  size_t _traceIdx = 0;

private:
  // kernel thread ID
  pid_t _tid;

  // The path of the thread folder
  std::string _thread_folder_name;
};

/**
 * TraCR Proc class, each MPI instance can how one.
 */
class TraCRProc {
public:
  /**
   * Constructor
   */
  TraCRProc(const pid_t &tid)
      : _tracr_init_time(NanoTimer::now()), _tid(tid), _lCPUid(sched_getcpu()) {

    _proc_folder_name = "proc." + std::to_string(_lCPUid) + "/";

    debug_print("_proc_folder_name: %s", _proc_folder_name.c_str());
  };

  /**
   * No default constructor allowed.
   */
  TraCRProc() = delete;

  /**
   * Default Destructor as we obey RAII
   */
  ~TraCRProc() = default;

  /**
   *
   */
  inline bool create_folder_recursive(const std::string &path = "") {
    _proc_folder_name = path + "tracr/" + _proc_folder_name;

    std::istringstream iss(_proc_folder_name);
    std::string token;
    std::string current;

    // Handle leading slash
    if (!_proc_folder_name.empty() && _proc_folder_name[0] == '/') {
      current = "/";
    }

    while (std::getline(iss, token, '/')) {
      if (token.empty())
        continue;
      current += token + "/";

      if (mkdir(current.c_str(), 0755) != 0) {
        if (errno != EEXIST) {
          std::cerr << "mkdir failed for: " << current << " errno=" << errno
                    << " (" << std::strerror(errno) << ")\n";
          return false;
        }
      }
    }

    return true;
  }

  /**
   *
   */
  inline std::string getFolderPath() { return _proc_folder_name; }

  /**
   * Adding a new tracr thread
   *
   * This is thread save.
   */
  inline void addTraCRThread(pid_t tid) {
    // We have to lock this as this method can be called from multiple threads
    std::lock_guard<std::mutex> lock(_tracrThreadIDsMutex);
    _tracrThreadIDs.push_back(tid);
  }

  /**
   *
   */
  inline void eraseTraCRThread(const pid_t tid) {
    std::lock_guard<std::mutex> lock(_tracrThreadIDsMutex);

    auto it = std::find(_tracrThreadIDs.begin(), _tracrThreadIDs.end(), tid);

    if (it == _tracrThreadIDs.end()) {
      std::cerr << "Thread not found in tracr proc list!\n";
      std::exit(EXIT_FAILURE);
    }

    if (it == _tracrThreadIDs.begin()) {
      std::cerr
          << "It is NOT allowed to thread_finalize the TraCR Proc thread!\n";
      std::exit(EXIT_FAILURE);
    }

    _tracrThreadIDs.erase(it);
  }

  /**
   *
   */
  inline pid_t getTID() { return _tid; }

  /**
   *
   */
  inline void addCustomChannelNames(const nlohmann::json &channel_names) {
    _json_file["channel_names"] = channel_names;
    _json_file["num_channels"] = channel_names.size();
  }

  /**
   *
   */
  inline void addNumberOfChannels(const u_int16_t num_channels) {
    _json_file["num_channels"] = num_channels;
  }

  /**
   *
   */
  inline void write_JSON() {
    // Before dumping, we have to fill in the metadata in the _json_file
    _json_file["pid"] = _lCPUid;
    _json_file["tid"] = _tid;
    _json_file["start_time"] = _tracr_init_time;

    for (const auto &[key, value] : _markerTypes) {
      _json_file["markerTypes"][std::to_string(key)] = value;
    }

    json_is_ready = true;
  }

  /**
   *
   */
  inline void dump_JSON() {
    if (!json_is_ready) {
      write_JSON();
    }

    // Create and open the metadata.json file
    std::string filename = _proc_folder_name + "metadata.json";
    std::ofstream file(filename);

    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << filename << " for writing!\n";
      std::exit(EXIT_FAILURE);
    }

    // Dump JSON into file (pretty-printed with 4 spaces)
    file << _json_file.dump(4);

    // Close the file
    file.close();

    debug_print("'%s' successfully written!", filename.c_str());
  }

  // A list of all the created _tracrThreadIDs
  // Publicly available as the list has to be checked
  // The first tracr Thread represents the one for the proc.
  // Every other is extra.
  std::vector<pid_t> _tracrThreadIDs;

  // The dynamic list to store all the marker types created
  std::unordered_map<uint16_t, std::string> _markerTypes;

  // Metadata and channel informations of this system
  nlohmann::json _json_file;

private:
  // TraCR start time
  int64_t _tracr_init_time;

  // kernel thread ID
  pid_t _tid;

  // logical CPU ID
  int _lCPUid;

  // A way to keep the tracrThreads save when adding/erasing
  std::mutex _tracrThreadIDsMutex;

  // The name of the proc folder
  std::string _proc_folder_name;

  //
  bool json_is_ready = false;
};

} // namespace TraCR