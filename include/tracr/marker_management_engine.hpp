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
#include <ctime>                //
#include <fstream>              // To store files
#include <filesystem>
#include <iomanip>
#include <unordered_map>
#include <unistd.h>             // SYS_gettid
#include <sys/syscall.h>        // syscall()
#include <sys/stat.h>           // mkdir()
#include <sys/types.h>          // chmod type
#include <sched.h>              // sched_getcpu()
#include <nlohmann/json.hpp>

#include <iostream>             // Debugging purposes

/**
 * The maximum capacity of one tracr thread for capturing the traces.
 * Currently, we fix it here. Might be definable by the user.
 * 
 * capatity = 2**16 = 65'536     -> 1MB tracr thread size
 * capacity = 2**20 = 1'048'576  -> 17MB tracr thread size
 * capacity = 2**24 = 16'777'216 -> 268MB tracr thread size
 */
constexpr size_t CAPACITY = 1 << 20;

/**
 * A way to check if the TraCRProc has been initialized, if not, throw at runtime.
 */
inline std::atomic<bool> tracr_proc_init{false};

/**
 * A way to enable/disable tracr at runtime
 */
inline std::atomic<bool> enable_tracr{true};

/**
 * 
 */
inline std::atomic<uint16_t> lazy_colorId{23};

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
    TraCRThread(pid_t tid) : _tid(tid) {};


    /**
     * Default Destructor as we obey RAII 
     */
    TraCRThread() = delete;
    ~TraCRThread() = default;

    /**
     * Store a given trace in Payload format
     */
    inline void store_trace(const Payload& payload) {
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
        if (std::filesystem::create_directories(_thread_folder_name) == 0) {
            std::cerr << "mkdir failed for: " << _thread_folder_name << "\n";
            std::exit(EXIT_FAILURE);
        }
        
        std::string filepath = _thread_folder_name + "traces.bts";

        std::cout << "The filepath of this TraCR thread[" << _tid << "] is: " << filepath << "\n";

        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs) {
            std::cerr << "Failed to open file: " << filepath << "\n";
            std::exit(EXIT_FAILURE);
        }

        // Write raw memory
        ofs.write(reinterpret_cast<const char*>(_traces.data()), sizeof(Payload) * _traceIdx);
        // return ofs.good();
    }

    /**
     * 
     */
    inline pid_t getTID() {
        return _tid;
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
    TraCRProc(pid_t tid) : _tracr_init_time(NanoTimer::now()), _tid(tid), _lCPUid(sched_getcpu()) {

        tracr_proc_init = true;
        
        _proc_folder_name = "proc." + std::to_string(_lCPUid) + "/";

        std::cout << "_proc_folder_name: " << _proc_folder_name << "\n";
    };

    /**
     * Default Con-/Destructor as we obey RAII 
     */
    TraCRProc() = delete;
    ~TraCRProc() = default;

    /**
     * 
     */
    inline bool create_folder_recursive(const std::string& path = "", mode_t mode = 0755) {
        
        _proc_folder_name = path + "tracr/" + _proc_folder_name;
        return std::filesystem::create_directories(_proc_folder_name);
    }

    /**
     * 
     */
    inline std::string getFolderPath() {
        return _proc_folder_name;
    }

    /**
     * Adding a new tracr thread
     * 
     * This is thread save.
     */
    inline void addTraCRThread(pid_t tid) {
        // We have to lock this as this method can be called from multiple threads
        std::lock_guard<std::mutex> lock(mtx);
        _tracrThreadIDs.push_back(tid);
    }

    /**
     * 
     */
    inline pid_t getTID() {
        return _tid;
    }

    /**
     * 
     */
    inline void addCustomChannelNames(const nlohmann::json& channel_names) {
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
    inline void dump_JSON() {
        // Before dumping, we have to fill in the metadata in the _json_file
        _json_file["pid"] = _lCPUid;
        _json_file["tid"] = _tid;
        _json_file["start_time"] = _tracr_init_time;

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

        std::cout << filename <<" successfully written!\n";
    }
    
    // A list of all the created _tracrThreadIDs
    // Publicly available as the list has to be checked
    // The first tracr Thread represents the one for the proc. 
    // Every other is extra.
    std::vector<pid_t> _tracrThreadIDs;

    // The dynamic list to store all the marker types created
    std::unordered_map<uint16_t, std::string> markerTypes;

    private:

    // TraCR start time
    int64_t _tracr_init_time;

    // kernel thread ID
    pid_t _tid;
    
    // logical CPU ID
    int _lCPUid;

    // A way to keep the tracrThreads save when adding to this class
    std::mutex mtx;
    
    // The name of the proc folder
    std::string _proc_folder_name;

    // Metadata and channel informations of this system
    nlohmann::json _json_file;
};