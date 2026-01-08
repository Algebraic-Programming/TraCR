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
#include <string>
#include <atomic>
#include <mutex>
#include <chrono>
#include <ctime>            //
#include <sstream>
#include <fstream>          // To store files
#include <iomanip>
#include <unistd.h>         // SYS_gettid
#include <sys/syscall.h>    // syscall()
#include <sys/stat.h>       // mkdir()
#include <sys/types.h>      // chmod type
#include <sched.h>          // sched_getcpu()

#include <iostream>         // Debugging purposes

/**
 * The maximum capacity of one tracr thread for capturing the traces.
 * Currently, we fix it here. Might be definable by the user.
 * 
 * capatity = 2**16 -> 1MB tracr thread size
 * capacity = 2**20 -> 17MB tracr thread size
 * capacity = 2**24 -> 268MB tracr thread size
 */
constexpr size_t CAPACITY 1<<16;

/**
 * A way to check if the TraCRProc has been initialized, if not, throw at runtime.
 */
inline std::atomic<bool> tracr_proc_init{false};

/**
 * Struct to capture the time stamp of the system
 */
struct timespec {
    time_t tv_sec;   // seconds (usually signed)
    long   tv_nsec;  // nanoseconds (0..999,999,999)
};

/**
 * 
 */
class NanoTimer {
public:
    // get current time in nanoseconds
    static uint64_t now() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL
             + static_cast<uint64_t>(ts.tv_nsec);
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

    // extraId consists of an extra information that has been added to be stored as well (i.e. The type of task label of the event type)
    uint32_t extraId;

    // Chrono nanosecond timestamp
    uint64_t timestamp;
};

/**
 * 
 */
class TraCRThread {
    public:

    /**
     * Constructor
     */
    TraCRThread() : _tid(syscall(SYS_gettid)) {};

    /**
     * Default Destructor as we obey RAII 
     */
    ~TraCRThread = default;

    /**
     * Store a given trace in Payload format
     */
    inline void store_trace(Payload payload) {
        _traces[_traceIdx % CAPACITY] = payload;
        ++_traceIdx;

        if(_traceIdx >= CAPACITY) {
            std::cerr << "Warning: TID[]" << _tid << " already overflowed the capacatity of available traces to store. It now overwrites from the beginning\n";
        }
    }

    /**
     * Flushed the traces into a file at the given path
     */
    inline void flush_traces(const std::string& path) {

        _thread_folder_name = path + "thread." + std::to_string(_tid) + "/";
        
        // create the last thread ID folder
        if (mkdir(_thread_folder_name.c_str(), 755) != 0) {
            if (errno != EEXIST) {
                std::cerr << "mkdir failed for: " << _thread_folder_name << "\n";
                std::exit(EXIT_FAILURE);
            }
        }
        
        std::string filepath = _thread_folder_name + "traces.bts";

        std::cout << "The filepath of this TraCR thread[" << _tid << "] is: " << filepath << "\n";

        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs) {
            std::cerr << "Failed to open file: " << filepath << "\n";
            return false;
        }

        // Write raw memory
        ofs.write(reinterpret_cast<const char*>(_traces.data()), sizeof(Payload) * _traceIdx);
        return ofs.good();
    }

    private:

    // The array to keep track of the traces
    std::array<Payload, CAPACITY> _traces;

    // The index at which point to add the next marker
    size_t _traceIdx = 0;

    // kernel thread ID
    pid_t _tid;

    // The path of the thread folder
    std::string _thread_folder_name;
};

/**
 * 
 */
class TraCRProc {
    public:

    /**
     * Constructor
     */
    TraCRProc : _tracr_init_time(NanoTimer::now()), _tid(syscall(SYS_gettid)), _lCPUid(sched_getcpu()) {

        tracr_proc_init = true;

        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;

        #ifdef _WIN32
        localtime_s(&tm_now, &t);   // Windows
        #else
        localtime_r(&t, &tm_now);   // Linux/Unix
        #endif

        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << tm_now.tm_year + 1900
            << std::setw(2) << std::setfill('0') << tm_now.tm_mon + 1
            << std::setw(2) << std::setfill('0') << tm_now.tm_mday
            << std::setw(2) << std::setfill('0') << tm_now.tm_hour;

        std::string date = oss.str();

        _proc_folder_name = date + "/proc." + std::to_string(_lCPUid) + "/";

        std::cout << "_proc_folder_name: " << _proc_folder_name << "\n";
    };

    /**
     * Default Destructor as we obey RAII 
     */
    ~TraCRProc = default;

    /**
     * 
     */
    inline bool create_folder_recursive(const std::string& path = "", mode_t mode = 0755) {
        size_t pos = 0;
        bool success = true;

        _proc_folder_name = path + "tracr/" + _proc_folder_name;

        std::cout << "creating a folder of the name: " << _proc_folder_name << "\n";

        while (true) {
            pos = _proc_folder_name.find('/', pos + 1);
            std::string subdir = _proc_folder_name.substr(0, pos);

            if (!subdir.empty()) {
                if (mkdir(subdir.c_str(), mode) != 0) {
                    if (errno != EEXIST) {
                        std::cerr << "mkdir failed: " << subdir << "\n";
                        success = false;
                        break;
                    }
                }
            }

            if (pos == std::string::npos) break; // done
        }

        return success;
    }

    /**
     * 
     */
    inline void addTraCRThread(std::unique_ptr<TraCRThread> t) {
        // We have to lock this as this method can be called from multiple threads
        std::lock_guard<std::mutex> lock(mtx);
        threads.push_back(std::move(t));
    }

    /**
     * 
     */
    inline pid_t getTID() {
        return _tid;
    }

    private:

    // TraCR start time
    int64_t _tracr_init_time;

    // kernel thread ID
    pid_t _tid;
    
    // logical CPU ID
    int _lCPUid;

    // A list of all the created _tracrThreads
    // Warning this might grow dynamically which could be not practical for a Ascend device.
    std::vector<std::unique_ptr<TraCRThread>> _tracrThreads;

    // A way to keep the tracrThreads save when adding to this class
    std::mutex mtx;
    
    // The name of the proc folder
    std::string _proc_folder_name;
};