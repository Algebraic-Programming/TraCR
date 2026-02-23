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

#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>

#include <tracr/tracr.hpp>

#define NRANKS 4 // number of threads
#define NTASKS 2 // number of tasks per thread

// globali accessible variables
uint16_t task_running_id;
uint16_t task_finished_id;

// Define a mutex
pthread_mutex_t printMutex;

// Function to be executed by a thread
void *threadFunction(void *arg) {

  uint16_t id = static_cast<uint16_t>(*(int *)arg);

  // TraCR init thread
  INSTRUMENTATION_THREAD_INIT();

  // Get the process ID (PID) and thread ID (TID)
  pid_t pid = getpid();            // Process ID
  pid_t tid = syscall(SYS_gettid); // Thread ID

  // Lock the mutex before printing
  pthread_mutex_lock(&printMutex);
  std::cout << "Thread " << id << " is running." << " PID: " << pid
            << ", TID: " << tid << std::endl;
  // Unlock the mutex after printing
  pthread_mutex_unlock(&printMutex);

  // running tasks
  for (int i = 0; i < NTASKS; ++i) {
    uint32_t taskid = id * NTASKS + i;

    INSTRUMENTATION_MARK_SET(id, task_running_id, taskid);

    std::cout << "Thread " << id << " is running task: " << taskid << std::endl;

    INSTRUMENTATION_MARK_SET(id, task_finished_id, taskid);
  }

  INSTRUMENTATION_MARK_RESET(id);

  // TraCR free thread
  INSTRUMENTATION_THREAD_FINALIZE();

  return nullptr;
}

/**
 * This is an example of using the thread markers combined with e.g. Pthreads
 */
int main() {
  // Initialize TraCR
  INSTRUMENTATION_START();

  task_running_id =
      INSTRUMENTATION_MARK_W_COLOR_ADD("task running", MARK_COLOR_MINT);
  task_finished_id = INSTRUMENTATION_MARK_ADD("task finishing");

  std::vector<pthread_t> threads(NRANKS); // Vector to hold pthreads
  std::vector<int> threadIds(NRANKS);     // Vector to hold thread IDs

  // Initialize the mutex
  pthread_mutex_init(&printMutex, nullptr);

  // Create NRANKS number of threads
  for (int i = 0; i < NRANKS; ++i) {
    threadIds[i] = i; // Assign thread ID
    pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
  }

  // Wait for all threads to finish
  for (int i = 0; i < NRANKS; ++i) {
    pthread_join(threads[i], nullptr);
  }

  std::cout << "All threads have finished." << std::endl;

  // Destroy the mutex
  pthread_mutex_destroy(&printMutex);

  // User-defined channels names to visualize
  nlohmann::json j = nlohmann::json::array();
  for (int i = 0; i < NRANKS; ++i) {
    j.push_back(std::string("Thread_") + std::to_string(i));
  }
  INSTRUMENTATION_ADD_CHANNEL_NAMES(j);

  // TraCR finished
  INSTRUMENTATION_END();

  return 0;
}
