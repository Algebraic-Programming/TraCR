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

	printf("task(%d) executed\n", taskid);

	INSTRUMENTATION_TASK_SUSPEND(taskid);

	printf("task(%d) suspended\n", taskid);

	INSTRUMENTATION_TASK_NOTADD(taskid);

	printf("task(%d) not added\n", taskid);

	INSTRUMENTATION_TASK_ADD(taskid);

	printf("task(%d) added\n", taskid);

	INSTRUMENTATION_TASK_READY(taskid);

	printf("task(%d) ready\n", taskid);

	INSTRUMENTATION_TASK_FINISH(taskid);

	printf("task(%d) finishing\n", taskid);

	INSTRUMENTATION_TASK_END(taskid);

	printf("task(%d) ended\n", taskid);

	INSTRUMENTATION_END();

	return 0;
}
