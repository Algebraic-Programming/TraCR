#include <iostream>
#include <pthread.h>
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>
#include <atomic>       // atomic counter to keep track how many tasks go executed

#include <detectr.hpp>

#define NRANKS 4  // number of threads
#define NTASKS 4  // number of tasks per thread

// Define a mutex
pthread_mutex_t printMutex;

// global task counter
std::atomic<uint32_t> task_counter(0);

// Function to be executed by a thread
void* threadFunction(void* arg) {

    int id = *(int*)arg;
    
    // ovni init thread
    INSTRUMENTATION_THREAD_INIT();

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
        uint32_t taskid = task_counter.fetch_add(1);

        INSTRUMENTATION_TASK_EXEC(taskid);

        std::cout << "Thread " << id << " is running task: " << taskid << std::endl;

        INSTRUMENTATION_TASK_END(taskid);
    }


    // ovni free thread
    INSTRUMENTATION_THREAD_END();

    return nullptr;
}

int main() {
    // ovni proc init
    INSTRUMENTATION_START();

    INSTRUMENTATION_REQUIRE_TASKR();

    int nranks = 4;  // You can change this to create any number of threads
    std::vector<pthread_t> threads(nranks);  // Vector to hold pthreads
    std::vector<int> threadIds(nranks);      // Vector to hold thread IDs

    // Initialize the mutex
    pthread_mutex_init(&printMutex, nullptr);

    // Create nranks number of threads
    for (int i = 0; i < nranks; ++i) {
        threadIds[i] = i + 1;  // Assign thread ID starting from 1
        pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < nranks; ++i) {
        pthread_join(threads[i], nullptr);
    }

    std::cout << "All threads have finished." << std::endl;

    // Destroy the mutex
    pthread_mutex_destroy(&printMutex);

    /* Write ntasks in metadata */
	INSTRUMENTATION_SET_NTASKS(task_counter.load());

    // ovni free proc
    INSTRUMENTATION_END();

    return 0;
}
