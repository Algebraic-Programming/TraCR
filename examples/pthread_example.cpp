#include <iostream>
#include <pthread.h>
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>

#include <tracr.hpp>

#define NRANKS 4  // number of threads
#define NTASKS 3  // number of tasks per thread

// globali accessible variables
size_t thrd_running_id;
size_t thrd_finished_id;

// Define a mutex
pthread_mutex_t printMutex;

// Function to be executed by a thread
void* threadFunction(void* arg) {

    int id = *(int*)arg;
    
    // ovni init thread
    INSTRUMENTATION_THREAD_INIT();

    INSTRUMENTATION_THREAD_MARK_SET(thrd_running_id);

    // Get the process ID (PID) and thread ID (TID)
    pid_t pid = getpid();              // Process ID
    pid_t tid = syscall(SYS_gettid);   // Thread ID

    // Lock the mutex before printing
    pthread_mutex_lock(&printMutex);
    std::cout << "Thread " << id << " is running."
              << " PID: " << pid << ", TID: " << tid << std::endl;
    // Unlock the mutex after printing
    pthread_mutex_unlock(&printMutex);

    // running tasks
    for(int i = 0; i < NTASKS; ++i) {
        uint32_t taskid = id*NTASKS + i;

        std::cout << "Thread " << id << " is running task: " << taskid << std::endl;
    }

    INSTRUMENTATION_THREAD_MARK_SET(thrd_finished_id);

    // ovni free thread
    INSTRUMENTATION_THREAD_END();

    return nullptr;
}

int main() {
    // ovni proc init
    INSTRUMENTATION_START();

    /**
	 * 0 == Set (Default) and 1 == Push/Pop
	 */
    INSTRUMENTATION_THREAD_MARK_INIT(0);

    thrd_running_id = INSTRUMENTATION_THREAD_MARK_ADD(MARK_COLOR_MINT, "thread running");
    thrd_finished_id = INSTRUMENTATION_THREAD_MARK_ADD(MARK_COLOR_GREEN, "thread finished");
    const size_t thrd_join_id = INSTRUMENTATION_THREAD_MARK_ADD(MARK_COLOR_BROWN, "join threads");
    const size_t thrds_end_id = INSTRUMENTATION_THREAD_MARK_ADD(MARK_COLOR_RED, "threads end");

    std::vector<pthread_t> threads(NRANKS);  // Vector to hold pthreads
    std::vector<int> threadIds(NRANKS);      // Vector to hold thread IDs

    // Initialize the mutex
    pthread_mutex_init(&printMutex, nullptr);

    // Create NRANKS number of threads
    for (int i = 0; i < NRANKS; ++i) {
        threadIds[i] = i;  // Assign thread ID
        pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
    }

    INSTRUMENTATION_THREAD_MARK_SET(thrd_join_id);

    // Wait for all threads to finish
    for (int i = 0; i < NRANKS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    INSTRUMENTATION_THREAD_MARK_SET(thrds_end_id);

    std::cout << "All threads have finished." << std::endl;

    // Destroy the mutex
    pthread_mutex_destroy(&printMutex);

    // ovni free proc
    INSTRUMENTATION_END();

    return 0;
}
