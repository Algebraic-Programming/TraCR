/* Copyright (c) 2021-2023 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: MIT */

#ifndef COMPAT_H
#define COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

pid_t get_tid(void);
int sleep_us(long usec);

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_H */
