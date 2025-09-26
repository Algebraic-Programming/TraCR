/*
 *   Copyright 2025 Huawei Technologies Co., Ltd.
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
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>

#include <tracr/tracr.hpp>

#define NRANKS 4 // number of threads
#define NTASKS 3 // number of tasks per thread

// globali accessible variables
size_t task_running_id;
size_t task_finished_id;
size_t thrd_running_id;
size_t thrd_finished_id;

// Define a mutex
pthread_mutex_t printMutex;

// Function to be executed by a thread
void *threadFunction(void *arg) {

  int id = *(int *)arg;

  // TraCR init thread
  INSTRUMENTATION_THREAD_INIT();

  INSTRUMENTATION_MARK_SET(thrd_running_id);

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

    INSTRUMENTATION_TASK_INIT(); // always init first

    INSTRUMENTATION_TMARK_SET(taskid, task_running_id);

    std::cout << "Thread " << id << " is running task: " << taskid << std::endl;

    INSTRUMENTATION_TMARK_SET(taskid, task_finished_id);
  }

  INSTRUMENTATION_MARK_SET(thrd_finished_id);

  // Optional: This marker pushes the int64_t max value. Can be used to indicate
  // the ending (which also makes it in Paraver by default invinsible).
  INSTRUMENTATION_MARK_RESET();

  // TraCR free thread
  INSTRUMENTATION_THREAD_END();

  return nullptr;
}

/**
 * This is an example of using the thread markers combined with e.g. Pthreads
 */
int main() {
  // Initialize TraCR
  // This boolean is a check to see if ovni has been initialize by another
  // library (e.g. nOS-V)
  bool externally_init = false;
  INSTRUMENTATION_START(externally_init);

  // 0 == Set and 1 == Push/Pop
  INSTRUMENTATION_TMARK_INIT(0);

  task_running_id = INSTRUMENTATION_TMARK_ADD(MARK_COLOR_MINT, "task running");
  task_finished_id = INSTRUMENTATION_TMARK_LAZY_ADD("task finished");

  INSTRUMENTATION_MARK_INIT(0);

  // Each Label creation costs around (~3us)
  // Should be done at the beginning or at the ending of the code
  thrd_running_id = INSTRUMENTATION_MARK_LAZY_ADD("thread running");
  thrd_finished_id =
      INSTRUMENTATION_MARK_ADD(MARK_COLOR_GREEN, "thread finished");
  const size_t thrd_join_id =
      INSTRUMENTATION_MARK_ADD(MARK_COLOR_BROWN, "join threads");
  const size_t thrds_end_id =
      INSTRUMENTATION_MARK_ADD(MARK_COLOR_RED, "threads end");

  std::vector<pthread_t> threads(NRANKS); // Vector to hold pthreads
  std::vector<int> threadIds(NRANKS);     // Vector to hold thread IDs

  // Initialize the mutex
  pthread_mutex_init(&printMutex, nullptr);

  // Create NRANKS number of threads
  for (int i = 0; i < NRANKS; ++i) {
    threadIds[i] = i; // Assign thread ID
    pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
  }

  INSTRUMENTATION_MARK_SET(thrd_join_id);

  // Wait for all threads to finish
  for (int i = 0; i < NRANKS; ++i) {
    pthread_join(threads[i], nullptr);
  }

  INSTRUMENTATION_MARK_SET(thrds_end_id);

  std::cout << "All threads have finished." << std::endl;

  // Destroy the mutex
  pthread_mutex_destroy(&printMutex);

  // TraCR finished
  INSTRUMENTATION_END();

  return 0;
}
