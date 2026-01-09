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

#include <unistd.h>             // SYS_gettid
#include <sys/syscall.h>        // syscall()
#include <string>
#include <nlohmann/json.hpp>

#include "marker_management_engine.hpp"

/**
 * Global TraCR proc place holder
 */
static inline std::unique_ptr<TraCRProc> tracrProc;

/**
 * Global TraCR thread place holder
 */
static inline thread_local std::unique_ptr<TraCRThread> tracrThread;

/**
 * 
 */
static inline void instrumentation_start(const std::string& path = "") {
    
    // This could be also checked with if(!tracrProc){} but would not be thread safe
    if(tracr_proc_init.load()){
        std::cerr << "TraCR Proc has already been initialized by the thread: " << tracrProc.getTID() << "\n";
        std::exit(EXIT_FAILURE);
    } 

    // Initialize the TraCRProc
    tracrProc = std::make_unique<TraCRProc>();

    // Create the folders to store the traces
    if(!tracrProc.create_folder_recursive(path)) {
        std::cerr << "Folder creation did not work: " << path << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Get current thread ID
    pid_t tid = syscall(SYS_gettid);

    // Add tracr Thread
    tracrThread = std::make_unique<TraCRThread>(tid);

    // Add its TraCRThread TID
    tracrProc.addTraCRThread(tid);

    // Create the folder for the TraCR Threads for later
    tracrProc.create_folder_recursive(path);
}

/**
 * 
 */
static inline void instrumentation_end(const uint16_t& num_channels = 0, const nlohmann::json& channel_names = json{}) {
    if(!tracr_proc_init.load()){
        std::cerr << "TraCR Proc has not been initialized by the thread: " << tracrProc.getTID() << "\n";
        std::exit(EXIT_FAILURE);
    }

    if(tracrProc._tracrThreads.size() != 1) {
        std::cerr << "TraCR Proc should only have his thread left but got: " << tracrProc._tracrThreads.size() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Get current thread ID
    pid_t tid = syscall(SYS_gettid);

    if(tracrProc._tracrThreads[0] != tid) {
        std::cerr << "TraCR instrumentation_end called by thread: " << tid << " instead of the main thread: " << tracrProc._tracrThreads[0] <<"\n";
        std::exit(EXIT_FAILURE);
    }

    // Set the global boolean back to not being initialized
    tracr_proc_init = false;

    // Destroys the TraCR Thread pointer and calls the destructor
    tracrThread.reset();

    // Dump Custom channel names OR number of channels to visualize (NOT BOTH)
    if(!channel_names.empty()){
        tracrProc.addCustomChannelNames(channel_names);
    }else if(num_channels > 0){
        tracrProc.addNumberOfChannels(num_channels)
    }else{
        std::cerr << "TraCR needs either the custom channel names or the number of channels to visualize\n";
        std::exit(EXIT_FAILURE);
    }
    
    // Destroys the TraCR Proc pointer and calls the destructor
    tracrProc.reset();
}

/**
 * 
 */
static inline void instrumentation_thread_init() {
    // Check 
    if(tracrThread){
        std::cerr << "TraCR Thread already exists with TID: " << tracrThread.getTID() << "\n";
        std::exit(EXIT_FAILURE);
    }

    pid_t tid = syscall(SYS_gettid);

    for(const auto& t : tracrProc._tracrThreads) {
        if(tid == t.getTID()) {
            std::cerr << "TraCR thread with this TID already exists in the list in tracr proc\n";
            std::exit(EXIT_FAILURE);
        }
    }

    // Add tracr Thread
    tracrThread = std::make_unique<TraCRThread>(tid);

    // Add the new TraCR Thread
    tracrProc.addTraCRThread(tid);
}

/**
 * 
 */
static inline void instrumentation_thread_finalize() {
    // Check if the tracr thread exists
    if(!tracrThread){
        std::cerr << "TraCR Thread doesn't exist\n";
        std::exit(EXIT_FAILURE);
    }

    // If it exists check if it is inside the tracr proc list
    pid_t tid = syscall(SYS_gettid);
    bool is_in_list = false;
    for (auto it = tracrProc._tracrThreads.begin(); it != tracrProc._tracrThreads.end(); ++it) {
        
        if(tid == (*it).getTID()) {
            if(it == tracrProc._tracrThreads.begin()) {
                std::cerr << "It is NOT allowed to thread_finalize the TraCR Proc thread!\n";
                std::exit(EXIT_FAILURE);
            }

            is_in_list = true;
            tracrProc._tracrThreads.erase(it);
            break;
        }
    }

    if(!is_in_list) {
        std::cerr << "TraCR Thread is not inside the TraCR Proc list\n";
        std::exit(EXIT_FAILURE);
    }

    // Flushing the trace of this TraCR thread now
    tracrThread.flush_traces(tracrProc.getFolderPath());

    // Finalize the thread now
    tracrThread.reset();
}

/**
 * 
 */
static inline void instrumentation_mark_add() {

}

/**
 * 
 */
static inline void instrumentation_mark_set() {

}

/**
 * 
 */
static inline void instrumentation_mark_reset() {

}

/**
 * 
 */
static inline void instrumentation_on() {

}

/**
 * 
 */
static inline void instrumentation_off() {

}