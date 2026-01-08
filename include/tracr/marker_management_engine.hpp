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
#include <iomanip>
#include <unistd.h>         // SYS_gettid
#include <sys/syscall.h>    // syscall()
#include <sys/stat.h>       // mkdir()
#include <sys/types.h>      // chmod type
#include <sched.h>          // sched_getcpu()

#include <iostream>         // Debugging purposes

/**
 * The maximum capacity of one tracr thread for capturing the traces
 * 
 * capatity = 2**16 -> 1MB tracr thread size
 * capacity = 2**20 -> 17MB tracr thread size
 * capacity = 2**24 -> 268MB tracr thread size
 */
constexpr size_t CAPACITY 1<<16;

/**
 * 
 */
inline std::atomic<bool> proc_init{false};

/**
 * Marker payload
 */
struct payload {
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

    // Constructor
    TraCRThread {

    };

    // Destructor
    ~TraCRThread = default;


    private:

    // The array to keep track of the markers
    std::array<payload, CAPACITY> _markers;

    // The index at which point to add the next marker
    size_t _markerIdx = 0;

    // kernel thread ID
    pid_t _tid;
};

/**
 * 
 */
class TraCRProc {
    public:

    /**
     * Constructor
     */
    TraCRProc {

        _lCPUid = sched_getcpu();

        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;

        #ifdef _WIN32
        localtime_s(&tm_now, &t);   // Windows
        #else
        localtime_r(&t, &tm_now);   // Linux/Unix
        #endif

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << tm_now.tm_mon + 1
            << std::setw(2) << std::setfill('0') << tm_now.tm_mday
            << std::setw(2) << std::setfill('0') << tm_now.tm_hour
            << std::setw(2) << std::setfill('0') << tm_now.tm_min
            << std::setw(2) << std::setfill('0') << tm_now.tm_sec;

        std::string _date = oss.str();

        _proc_folder_name = std::to_string(_lCPUid) + "/" + _date + "/";

        std::cout << "_proc_folder_name: " << _proc_folder_name << "\n";
    };

    /**
     * Destructor
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

    inline void addTraCRThread(std::unique_ptr<Thread> t) {
        std::lock_guard<std::mutex> lock(mtx);
        threads.push_back(std::move(t));
    }

    private:

    // A list of all the created _tracrThreads
    std::vector<std::unique_ptr<TraCRThread>> _tracrThreads;

    // A way to keep the tracrThreads save when adding to this class
    std::mutex mtx;
    
    // logical CPU number
    int _lCPUid;

    // The name of the proc folder
    std::string _proc_folder_name;
};