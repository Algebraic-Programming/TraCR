"""
    Copyright 2025 Huawei Technologies Co., Ltd.
 
  Licensed under the Apache License, Version 2.0 (the "License")
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
      http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
"""

import os

import ctypes

# SYS_gettid syscall number is 186 on x86_64
libc = ctypes.CDLL("libc.so.6")
libc.sched_getcpu.restype = ctypes.c_int
SYS_gettid = 186

from multiprocessing import Process, Lock

from tracr import *

NRANKS = 4 # number of threads
NTASKS = 3 # number of tasks per thread

# Function to be executed by a thread
def threadFunction(lock, id, thrd_running_id, thrd_finished_id):

  # TraCR init thread
  INSTRUMENTATION_THREAD_INIT()

  INSTRUMENTATION_THREAD_MARK_SET(thrd_running_id)

  # Get the process ID (PID) and thread ID (TID)
  pid = os.getpid() # Process ID
  CPUid = libc.sched_getcpu()

  # Lock for printing
  with lock:
    print(f"Thread {id} is running. CPUid: {CPUid}, PID: {pid}")


  # running tasks
  for i in range(NTASKS):
    taskid = id * NTASKS + i

    print(f"Thread {id} is running task: {taskid}")

  INSTRUMENTATION_THREAD_MARK_SET(thrd_finished_id)

  # TraCR free thread
  INSTRUMENTATION_THREAD_END()


"""
 This is an example of using the thread markers combined with e.g. Pthreads
"""
def main():
  # Initialize TraCR
  INSTRUMENTATION_START()

  # 0 == Set and 1 == Push/Pop
  INSTRUMENTATION_THREAD_MARK_INIT(0)

  # Each Label creation costs around (~3us)
  # Should be done at the beginning or at the ending of the code
  thrd_running_id = INSTRUMENTATION_THREAD_MARK_ADD(mark_color.MARK_COLOR_MINT, "thread running")
  thrd_finished_id = INSTRUMENTATION_THREAD_MARK_ADD(mark_color.MARK_COLOR_GREEN, "thread finished")
  thrd_join_id = INSTRUMENTATION_THREAD_MARK_ADD(mark_color.MARK_COLOR_BROWN, "join threads")
  thrds_end_id = INSTRUMENTATION_THREAD_MARK_ADD(mark_color.MARK_COLOR_RED, "threads end")

  threads = [None] * NRANKS   # Vector to hold pthreads
  threadIds = [None] * NRANKS # Vector to hold thread IDs

  # Initialize the lock
  lock = Lock()

  # Create NRANKS number of threads
  for i in range(NRANKS):
    threadIds[i] = i # Assign thread ID

    threads[i] = Process(target=threadFunction, args=(lock, i, thrd_running_id, thrd_finished_id))

    threads[i].start()

  INSTRUMENTATION_THREAD_MARK_SET(thrd_join_id)

  # Wait for all threads to finish
  for thread in threads:
    thread.join()

  INSTRUMENTATION_THREAD_MARK_SET(thrds_end_id)

  print("All threads have finished.")

  # TraCR finished
  INSTRUMENTATION_END()


if __name__ == "__main__":
   main()