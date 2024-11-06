#include <iostream>
#include <pthread.h>
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>

#include <ovni.h>
#include "common/instr.h"

// Define a mutex
pthread_mutex_t printMutex;

// Function to be executed by a thread
void* threadFunction(void* arg) {

    int id = *(int*)arg;
    
    // ovni init thread (id counting starts at zero!)
    instr_init_thread(id-1);

    // Get the process ID (PID) and thread ID (TID)
    pid_t pid = getpid();              // Process ID
    pid_t tid = syscall(SYS_gettid);   // Thread ID

    // Lock the mutex before printing
    pthread_mutex_lock(&printMutex);
    std::cout << "Thread " << id << " is running."
              << " PID: " << pid << ", TID: " << tid << std::endl;
    // Unlock the mutex after printing
    pthread_mutex_unlock(&printMutex);

    // ovni free thread
    instr_thread_end();
	ovni_thread_free();

    return nullptr;
}

int main() {
    // ovni proc init
    instr_init_proc();

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

    // ovni free proc
    ovni_proc_fini();

    return 0;
}
