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

#include <iostream>
#include <sched.h> // For sched_getcpu()
#include <string>
#include <sys/syscall.h> // For syscall(SYS_gettid)
#include <unistd.h>
#include <unistd.h> // For getpid()
#include <vector>

#include <ovni.h>

/**
 * Function to initialize a ovni thread
 */
static inline void thread_execute(int32_t cpu, int32_t ctid, uint64_t tag) {
  struct ovni_ev ev = {0};
  ovni_ev_set_clock(&ev, ovni_clock_now());
  ovni_ev_set_mcv(&ev, "OHx");
  ovni_payload_add(&ev, (uint8_t *)&cpu, sizeof(cpu));
  ovni_payload_add(&ev, (uint8_t *)&ctid, sizeof(ctid));
  ovni_payload_add(&ev, (uint8_t *)&tag, sizeof(tag));
  ovni_ev_emit(&ev);
}

/**
 * Function to return the current TID
 */
static inline pid_t get_tid(void) { return (pid_t)syscall(SYS_gettid); }

/**
 * Function to initialize the ovni proc (this has to be done only once)
 */
static inline void instrumentation_init_proc(int rank, int nranks) {
  char hostname[OVNI_MAX_HOSTNAME];
  char rankname[OVNI_MAX_HOSTNAME + 64];

  if (gethostname(hostname, OVNI_MAX_HOSTNAME) != 0) {
    std::cerr << "hostname to long: " << hostname << std::endl;
    std::exit(EXIT_FAILURE); // Exits with a failure status
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

  // For visualization convenience the instrumentation proc will be initialized
  // on the CPU ID -1. One can also initialize it to "rank" if wanted
  thread_execute(-1, get_tid(), 0);
}

/**
 * Initializing a ovni thread. Each new spawned thread has to call this to keep
 * track of his traces.
 */
static inline void instrumentation_init_thread() {
  ovni_thread_init(get_tid());

  thread_execute(sched_getcpu(), get_tid(), 0);
}

/**
 * Finalizing the ovni thread livespam. After this no more markers are allowed
 * on the same thread! Also, one can not re-initialize the same thread.
 */
static inline void instrumentation_thread_end(void) {
  struct ovni_ev ev = {0};

  ovni_ev_set_mcv(&ev, "OHe");
  ovni_ev_set_clock(&ev, (uint64_t)ovni_clock_now());
  ovni_ev_emit(&ev);

  /* Flush the events to disk before killing the thread */
  ovni_flush();
}

/**
 * The finalization call of ovni. This one only has to be called by the main
 * proc.
 */
static inline void instrumentation_end(void) {
  instrumentation_thread_end();
  ovni_thread_free();
  ovni_proc_fini();
}

/* Ovni Thread Markers */

/**
 * The class we use to store the colors in a vector the keep track the label
 * (int) This will let the user define their own label id's like this: const
 * size_t free_mem_label_id  = INSTRUMENTATION_MARKER_ADD("Free memory",
 * MARK_COLOR_MINT); This class is build very lightweight for performance. An
 * older version with storing the string exists in the 'task_more_states'
 * branch
 * https://gitlab.huaweirc.ch/zrc-von-neumann-lab/runtime-system-innovations/tracr/-/tree/task_more_states?ref_type=heads
 */
class ThreadMarkerMap {
public:
  /**
   * Store the ovni mark label color value in the vector.
   * NOTE: labelid (i.e. the color) has to be unique otherwise ovni will call an
   * error! Also the color can't be black (i.e. labelid == 0)
   */
  size_t add(int64_t labelid, const std::string &label) {

    ovni_mark_label(0, labelid, label.c_str());

    // Insert the corresponding integer labelid
    colors.push_back(labelid);

    return colors.size() - 1;
  }

  /**
   * ovni mark set call with the returned idx from the 'add' method
   */
  void set(size_t idx) { ovni_mark_set(0, colors[idx]); }

  /**
   * ovni mark push call with the returned idx from the 'add' method
   */
  void push(size_t idx) { ovni_mark_push(0, colors[idx]); }

  /**
   * ovni mark pop call with the returned idx from the 'add' method
   */
  void pop(size_t idx) { ovni_mark_pop(0, colors[idx]); }

private:
  std::vector<int64_t> colors;
};