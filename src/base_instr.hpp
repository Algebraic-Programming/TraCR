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

static inline void
instr_taskr_mark_create(uint32_t labelid, const char *label)
{
	struct ovni_ev ev = {0};
	ovni_ev_set_clock(&ev, (uint64_t) ovni_clock_now());
	ovni_ev_set_mcv(&ev, "tMc");

	// Convert uint32_t label to a string and append it to the original string
	char conc_label[strlen(label) + 32];

    // Concatenate the label to the original string with the unique char '$'. Should not be used by the user!
	sprintf(conc_label, "%s$%u", label, labelid);

	ovni_ev_jumbo_emit(&ev, (uint8_t *) conc_label, (uint32_t) strlen(conc_label));
}

#define INSTR_2ARG(name, mcv, ta, a, tb, b)                       \
	static inline void name(ta a, tb b)                       \
	{                                                         \
		struct ovni_ev ev = {0};                          \
		ovni_ev_set_clock(&ev, (uint64_t) ovni_clock_now());   \
		ovni_ev_set_mcv(&ev, mcv);                        \
		ovni_payload_add(&ev, (uint8_t *) &a, sizeof(a)); \
		ovni_payload_add(&ev, (uint8_t *) &b, sizeof(b)); \
		ovni_ev_emit(&ev);                                \
	}


INSTR_2ARG(instr_taskr_mark_set,  "tM=", uint32_t, taskid, uint32_t, labelid)
INSTR_2ARG(instr_taskr_mark_push, "tM[", uint32_t, taskid, uint32_t, labelid)
INSTR_2ARG(instr_taskr_mark_pop,  "tM]", uint32_t, taskid, uint32_t, labelid)

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


/* Ovni Task Markers */

class TaskMarkerMap {
public:
	/**
	 * Store the ovni mark label color value in the vector.
	 * NOTE: value (i.e. the color) has to be unique otherwise only will call an error!
	 */
    size_t add(uint32_t labelid, const std::string& label) {

		instr_taskr_mark_create(labelid, label.c_str());

		// Insert the corresponding integer value
		colors.push_back(labelid);

		return colors.size() - 1;
    }

	/**
	 * ovni mark set call with the returned idx from the 'add' method
	 */
	void set(uint32_t taskid, size_t idx) {
		instr_taskr_mark_set(taskid, colors[idx]);
	}

	/**
	 * ovni mark push call with the returned idx from the 'add' method
	 */
	void push(uint32_t taskid, size_t idx) {
		instr_taskr_mark_push(taskid, colors[idx]);
	}

	/**
	 * ovni mark pop call with the returned idx from the 'add' method
	 */
	void pop(uint32_t taskid, size_t idx) {
		instr_taskr_mark_pop(taskid, colors[idx]);
	}

private:
	std::vector<uint32_t> colors;
};

TaskMarkerMap task_marker_map; // defined globali

/* Ovni Thread Markers */

/**
 * The class we use to store the colors in a vector the keep track the label (int)
 * This will let the user define their own label id's like this:
 * const size_t free_mem_label_id  = INSTRUMENTATION_MARKER_ADD("Free memory", MARK_COLOR_MINT);
 * This class is build very lightweight for performance. An older version with storing the string exists in the newest 'task_more_states' branch
 * https://gitlab.huaweirc.ch/zrc-von-neumann-lab/runtime-system-innovations/detectr/-/tree/task_more_states?ref_type=heads
 */
class ThreadMarkerMap {
public:
	/**
	 * Store the ovni mark label color value in the vector.
	 * NOTE: value (i.e. the color) has to be unique otherwise only will call an error!
	 */
    size_t add(int64_t value, const std::string& label) {

		ovni_mark_label(0, value, label.c_str());

		// Insert the corresponding integer value
		colors.push_back(value);

		return colors.size() - 1;
    }

	/**
	 * ovni mark set call with the returned idx from the 'add' method
	 */
	void set(size_t idx) {
		ovni_mark_set(0, colors[idx]);
	}

	/**
	 * ovni mark push call with the returned idx from the 'add' method
	 */
	void push(size_t idx) {
		ovni_mark_push(0, colors[idx]);
	}

	/**
	 * ovni mark pop call with the returned idx from the 'add' method
	 */
	void pop(size_t idx) {
		ovni_mark_pop(0, colors[idx]);
	}

private:
	std::vector<int64_t> colors;
};

ThreadMarkerMap thread_marker_map; // define globali

// /**
//  * Add ovni mark with the given color value to the marker_map. 
//  * No duplicates allowed. 
//  * NOT THREAD SAFE! Only the main thread should handle this.
//  */
// static inline size_t 
// thread_marker_add(const std::string& label, int64_t value)
// {
// 	return thread_marker_map.add(str, value);
// }

// /**
//  * ovni mark set call with the returned idx from the 'add' method
//  */
// static inline void 
// thread_marker_set(const size_t& idx)
// {
// 	thread_marker_map.set(idx);
// }

// /**
//  * ovni mark push call with the returned idx from the 'add' method
//  */
// static inline void 
// thread_marker_push(const size_t& idx)
// {
// 	thread_marker_map.push(idx);
// }

// /**
//  * ovni mark pop call with the returned idx from the 'add' method
//  */
// static inline void 
// thread_marker_pop(const size_t& idx)
// {
// 	thread_marker_map.pop(idx);
// }