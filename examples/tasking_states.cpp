/* Copyright (c) 2024 Barcelona Supercomputing Center (BSC)
 * SPDX-License-Identifier: GPL-3.0-or-later */

#include <detectr.hpp>

int
main(void)
{
	const uint32_t taskid = 0;

	INSTRUMENTATION_START();

	INSTRUMENTATION_TASK_INIT(taskid);

	printf("task(%d) initialized\n", taskid);

	INSTRUMENTATION_TASK_EXEC(taskid);

	printf("task(%d) executing\n", taskid);

	INSTRUMENTATION_TASK_PAUSE(taskid);

	printf("task(%d) paused\n", taskid);

	INSTRUMENTATION_TASK_SYNC(taskid);

	printf("task(%d) syncing\n", taskid);

	INSTRUMENTATION_TASK_FINISH(taskid);

	printf("task(%d) finishing\n", taskid);

	INSTRUMENTATION_TASK_END(taskid);

	printf("task(%d) ended\n", taskid);

	INSTRUMENTATION_END();

	return 0;
}
