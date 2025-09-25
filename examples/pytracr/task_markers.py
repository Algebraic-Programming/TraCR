"""
    Copyright 2025 Huawei Technologies Co., Ltd.
 
  Licensed under the Apache License, Version 2.0 (the "License")
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
      http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
"""

from tracr import *

"""
 Basic python code to demonstrate pyTraCR's TASK MARKERS (TMARK)
 This example uses the TMARK SET method
 you can create a nice gradient plot in Paraver with it
"""
def main():
	ntasks = 2

	INSTRUMENTATION_START()

	"""
	0 == Set and 1 == Push/Pop
	"""
	INSTRUMENTATION_TMARK_MARK_INIT(0)

	task_init_idx = INSTRUMENTATION_TMARK_ADD(mark_color.MARK_COLOR_GRAY, "task initialized")
	task_exec_idx = INSTRUMENTATION_TMARK_ADD(mark_color.MARK_COLOR_LIGHT_GREEN, "task executed")
	task_susp_idx = INSTRUMENTATION_TMARK_ADD(mark_color.MARK_COLOR_LIGHT_GRAY, "task suspended")
	task_fini_idx = INSTRUMENTATION_TMARK_ADD(mark_color.MARK_COLOR_BLACK, "task finished")
	
	for taskid in range(ntasks):
		INSTRUMENTATION_TASK_INIT()

		INSTRUMENTATION_TMARK_SET(taskid, task_init_idx)

		print(f"task{taskid} initialized")

		# same value push (should not fail!)
		INSTRUMENTATION_TMARK_SET(taskid, task_init_idx)

		print(f"task{taskid} initialized")

		INSTRUMENTATION_TMARK_SET(taskid, task_exec_idx)

		print(f"task{taskid} executed")

		INSTRUMENTATION_TMARK_SET(taskid, task_susp_idx)

		print(f"task{taskid} suspended")

		INSTRUMENTATION_TMARK_SET(taskid, task_fini_idx)

		print(f"task{taskid} finishing")

	INSTRUMENTATION_END()

if __name__ == "__main__":
	main()