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

INSTR_1ARG(instr_taskr_task_execute, "tTx", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_end, "tTe", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_pause, "tTp", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_finish, "tTf", uint32_t, taskid)
INSTR_1ARG(instr_taskr_task_sync, "tTs", uint32_t, taskid)

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

