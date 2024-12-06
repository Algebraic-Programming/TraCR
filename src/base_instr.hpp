/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file base_instr.hpp
 * @brief Common ovni base functions
 * @author Noah Baumann
 * @date 11/11/2024
 */

#pragma once

#include <unistd.h>
#include <iostream>
#include <sys/syscall.h> // For syscall(SYS_gettid)
#include <unistd.h>		 // For getpid()
#include <string>
#include <vector>

#include <ovni.h>

#define INSTR_1ARG(name, mcv, ta, a)                              \
	static inline void name(ta a)                             \
	{                                                         \
		struct ovni_ev ev = {0};                          \
		ovni_ev_set_clock(&ev, (uint64_t) ovni_clock_now());   \
		ovni_ev_set_mcv(&ev, mcv);                        \
		ovni_payload_add(&ev, (uint8_t *) &a, sizeof(a)); \
		ovni_ev_emit(&ev);                                \
	}

INSTR_1ARG(instr_taskr_task_init, "tTi", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_execute, "tTx", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_end, "tTe", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_suspend, "tTs", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_finish, "tTf", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_notadd, "tTn", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_add, "tTa", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_ready, "tTr", uint32_t, taskid)

/**
 * Function taken from here:
 * https://ovni.readthedocs.io/en/master/user/runtime/#setup_metadata
 */
static inline void 
thread_execute(int32_t cpu, int32_t ctid, uint64_t tag)
{
    struct ovni_ev ev = {0};
    ovni_ev_set_clock(&ev, ovni_clock_now());
    ovni_ev_set_mcv(&ev, "OHx");
    ovni_payload_add(&ev, (uint8_t *) &cpu, sizeof(cpu));
    ovni_payload_add(&ev, (uint8_t *) &ctid, sizeof(ctid));
    ovni_payload_add(&ev, (uint8_t *) &tag, sizeof(tag));
    ovni_ev_emit(&ev);
}

/**
 * 
 */
static inline pid_t
get_tid(void)
{
	return (pid_t) syscall(SYS_gettid);
}


/**
 * 
 */
static inline void
instrumentation_init_proc(int rank, int nranks)
{
    char hostname[OVNI_MAX_HOSTNAME];
	char rankname[OVNI_MAX_HOSTNAME + 64];

	if (gethostname(hostname, OVNI_MAX_HOSTNAME) != 0){
		std::cerr << "hostname to long: " << hostname << std::endl;
        std::exit(EXIT_FAILURE);  // Exits with a failure status
	}

	sprintf(rankname, "%s.%d", hostname, rank);

	ovni_version_check();
	ovni_proc_init(1, rankname, getpid());
	ovni_thread_init(get_tid());

	if (nranks > 0)
		ovni_proc_set_rank(rank, nranks);

	/* All ranks inform CPUs */
	for (int i = 0; i < nranks; i++)
		ovni_add_cpu(i, i);

	thread_execute(rank, -1, 0);
}

/**
 * 
 */
static inline void
instrumentation_init_thread()
{
	ovni_thread_init(get_tid());

	thread_execute(-1, -1, 0);
}

/**
 * 
 */
static inline void
instrumentation_thread_end(void)
{
	struct ovni_ev ev = {0};

	ovni_ev_set_mcv(&ev, "OHe");
	ovni_ev_set_clock(&ev, (uint64_t) ovni_clock_now());
	ovni_ev_emit(&ev);

	/* Flush the events to disk before killing the thread */
	ovni_flush();
}

/**
 * 
 */
static inline void
instrumentation_end(void)
{
	instrumentation_thread_end();
	ovni_thread_free();
	ovni_proc_fini();
}


// Ovni Markers (user friendly)

/**
 * The class we use to store the strings with their given color (int)
 */
class StringIntegerMap {
public:
	/**
	 * This method has to be called first before any other methods.
	 * With the flag, define if the ovni marker method push/pop or set is prefered.
	 * Use flag == 1 for push/pop and flag != 1 for set
	 */
	void init(long flag) {
		ovni_mark_type(0, flag, "DetectR Thread Markers");
	}

	/**
	 * Store the ovni mark label if not yet existing in the vector.
	 * If existing, error not allowed
	 */
    void add(const std::string& str, int64_t value) {

        // Check if the string is not yet in the map (and in ovni)
		std::pair<bool, int> found_idx = find(str);
		bool found = found_idx.first;
		int idx = found_idx.second;

		if(!found) {

			ovni_mark_label(0, value, str.c_str());

        	// Insert the string with its corresponding integer value
        	labels.push_back(str);
			colors.push_back(value);
        } else {
			std::cerr << "Error: Illegal move, String '" << str 
						<< "' exists with color: " << idx <<  std::endl;

			print();

			return;
		}
    }

	/**
	 * ovni mark set call only if the string is present in the marker_map.
	 * Else, error
	 */
	void set(const std::string& str) {
		std::pair<bool, int> found_idx = find(str);
		bool found = found_idx.first;
		int idx = found_idx.second;

		if(!found) {
			std::cerr << "Error: String '" << str 
					  << "' not found in vector:" <<  std::endl;

			print();

			return;
		}

		ovni_mark_set(0, colors[idx]);
	}

	/**
	 * ovni mark push call only if the string is present in the marker_map.
	 * Else, error
	 */
	void push(const std::string& str) {
		std::pair<bool, int> found_idx = find(str);
		bool found = found_idx.first;
		int idx = found_idx.second;

		if(!found) {
			std::cerr << "Error: String '" << str 
					  << "' not found in vector:" <<  std::endl;

			print();

			return;
		}

		ovni_mark_push(0, colors[idx]);
	}

	/**
	 * ovni mark pop call only if the string is present in the marker_map.
	 * Else, error
	 */
	void pop(const std::string& str) {
		std::pair<bool, int> found_idx = find(str);
		bool found = found_idx.first;
		int idx = found_idx.second;

		if(!found) {
			std::cerr << "Error: String '" << str 
					  << "' not found in vector:" <<  std::endl;

			print();

			return;
		}

		ovni_mark_pop(0, colors[idx]);
	}

	/**
	 * Checking if the given string is already existing.
	 * If yes, return true and the idx
	 * If no, return false
	 */
	std::pair<bool, int> find(const std::string& str) const {
		for (size_t i = 0; i < labels.size(); i++) {
			if(labels[i] == str) {
				return std::pair<bool, int>(true, (int) i);
			}
		}
		return std::pair<bool, int>(false, -1);
	}
    
	/**
	 * prints out the labels with the corresponding colors.
	 * (for debugging)
	 */
    void print() const {
        for (size_t i = 0; i < labels.size(); i++) {
            std::cout << "label: " << labels[i] << ", color: " << colors[i] << std::endl;
        }
    }

private:
    std::vector<std::string> labels;
	std::vector<int64_t> colors;
};

StringIntegerMap marker_map; // define globali

/**
 * Initialize the marker map.
 * flag == 1 for push/pop and flag != 1 for set
 */
static inline void 
marker_init(long flag)
{
	marker_map.init(flag);
}

/**
 * Add ovni mark with the given color value to the marker_map. 
 * No duplicates allowed. 
 * NOT THREAD SAFE! Only the main thread should handle this.
 */
static inline void 
marker_add(const std::string& str, int64_t value)
{
	marker_map.add(str, value);
}

/**
 * Set ovni mark if the string exists in the marker_map, else error
 */
static inline void 
marker_set(const std::string& str)
{
	marker_map.set(str);
}

/**
 * Push ovni mark if the string exists in the marker_map, else error
 */
static inline void 
marker_push(const std::string& str)
{
	marker_map.push(str);
}

/**
 * Pop ovni mark if the string exists in the marker_map, else error
 */
static inline void 
marker_pop(const std::string& str)
{
	marker_map.pop(str);
}