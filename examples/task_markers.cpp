/* Copyright (c) 2024 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include <detectr.hpp>

int
main(void)
{
	const uint32_t ntasks = 2;

	INSTRUMENTATION_START();

	/**
	 * 0 == Set and 1 == Push/Pop
	 */
	INSTRUMENTATION_TASK_MARK_TYPE(0);

	const size_t task_init_idx = INSTRUMENTATION_TASK_ADD(MARK_COLOR_GRAY, "task initialized");
	const size_t task_exec_idx = INSTRUMENTATION_TASK_ADD(MARK_COLOR_LIGHT_GREEN, "task executed");
	const size_t task_susp_idx = INSTRUMENTATION_TASK_ADD(MARK_COLOR_LIGHT_GRAY, "task suspended");
	const size_t task_fini_idx = INSTRUMENTATION_TASK_ADD(MARK_COLOR_BLACK, "task finished");
	
	for(uint32_t taskid = 0; taskid < ntasks; taskid++) {

		INSTRUMENTATION_TASK_INIT();

		INSTRUMENTATION_TASK_SET(taskid, task_init_idx);

		printf("task(%d) initialized\n", taskid);

		INSTRUMENTATION_TASK_SET(taskid, task_exec_idx);

		printf("task(%d) executed\n", taskid);

		INSTRUMENTATION_TASK_SET(taskid, task_susp_idx);

		printf("task(%d) suspended\n", taskid);

		INSTRUMENTATION_TASK_SET(taskid, task_fini_idx);

		printf("task(%d) finishing\n", taskid);
	}

	INSTRUMENTATION_END();

	return 0;
}
