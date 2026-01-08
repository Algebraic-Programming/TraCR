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

#include <unistd.h>         // SYS_gettid
#include <sys/syscall.h>    // syscall()
#include <string>

#include "marker_management_engine.hpp"

/**
 * Global place holder
 */
static inline std::unique_ptr<TraCRProc> tracrProc;

/**
 * 
 */
static inline void instrumentation_start() {
    
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

    // Add its TraCRThread
    tracrProc.addTraCRThread(std::make_unique<TraCRThread>());

}

/**
 * 
 */
static inline void instrumentation_end(const std::string& path = "") {
    if(!tracr_proc_init.load()){
        std::cerr << "TraCR Proc has not been initialized by the thread: " << tracrProc.getTID() << "\n";
        std::exit(EXIT_FAILURE);
    }

    tracr_proc_init = false;
    
    // Destroys the pointer and calls the destructor
    tracrProc.reset();
}

/**
 * 
 */
static inline void instrumentation_thread_init() {
    
}

/**
 * 
 */
static inline void instrumentation_thread_finalize() {
    
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