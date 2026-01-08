/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file base_instr.hpp
 * @brief Common ovni base functions
 * @author Noah Andres Baumann
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

static inline void instr_taskr_mark_create(uint32_t labelid,
                                           const char *label) {
  struct ovni_ev ev = {0};
  ovni_ev_set_clock(&ev, (uint64_t)ovni_clock_now());
  ovni_ev_set_mcv(&ev, "tMc+");

  // Convert uint32_t label to a string and append it to the label string
  const size_t buf_size = sizeof(labelid) + strlen(label) + 1;

  if (buf_size > 512) {
    std::cerr << "label too long: " << label << std::endl;
    std::exit(EXIT_FAILURE);
  }

  char buf[buf_size];

  char *p = buf;

  size_t nbytes = 0;
  memcpy(buf, &labelid, sizeof(labelid));
  p += sizeof(labelid);
  nbytes += sizeof(labelid);
  sprintf(p, "%s", label);
  nbytes += strlen(p) + 1;

  ovni_ev_jumbo_emit(&ev, (uint8_t *)buf, (uint32_t)nbytes);
}

#define INSTR_2ARG(name, mcv, ta, a, tb, b)                                    \
  static inline void name(ta a, tb b) {                                        \
    struct ovni_ev ev = {0};                                                   \
    ovni_ev_set_clock(&ev, (uint64_t)ovni_clock_now());                        \
    ovni_ev_set_mcv(&ev, mcv);                                                 \
    ovni_payload_add(&ev, (uint8_t *)&a, sizeof(a));                           \
    ovni_payload_add(&ev, (uint8_t *)&b, sizeof(b));                           \
    ovni_ev_emit(&ev);                                                         \
  }

INSTR_2ARG(instr_taskr_mark_set, "tM=", uint32_t, taskid, uint32_t, labelid)
INSTR_2ARG(instr_taskr_mark_push, "tM[", uint32_t, taskid, uint32_t, labelid)
INSTR_2ARG(instr_taskr_mark_pop, "tM]", uint32_t, taskid, uint32_t, labelid)

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