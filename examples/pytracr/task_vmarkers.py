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
	INSTRUMENTATION_VTMARK_INIT(0)

	INSTRUMENTATION_VTMARK_ADD(mark_color.MARK_COLOR_GRAY, "task initialized")
	INSTRUMENTATION_VTMARK_ADD(mark_color.MARK_COLOR_LIGHT_GREEN, "task executed")
	INSTRUMENTATION_VTMARK_ADD(mark_color.MARK_COLOR_LIGHT_GRAY, "task suspended")
	INSTRUMENTATION_VTMARK_ADD(mark_color.MARK_COLOR_BLUE, "task finished")
	
	for taskid in range(ntasks):
		INSTRUMENTATION_TASK_INIT()

		INSTRUMENTATION_VTMARK_SET(taskid, mark_color.MARK_COLOR_GRAY)

		print(f"task{taskid} initialized")

		# same value push (should not fail!)
		INSTRUMENTATION_VTMARK_SET(taskid, mark_color.MARK_COLOR_GRAY)

		print(f"task{taskid} initialized")

		INSTRUMENTATION_VTMARK_SET(taskid, mark_color.MARK_COLOR_LIGHT_GREEN)

		print(f"task{taskid} executed")

		INSTRUMENTATION_VTMARK_SET(taskid, mark_color.MARK_COLOR_LIGHT_GRAY)

		print(f"task{taskid} suspended")

		INSTRUMENTATION_VTMARK_SET(taskid, mark_color.MARK_COLOR_BLUE)

		print(f"task{taskid} finishing")

		INSTRUMENTATION_TMARK_RESET(taskid)

	json_str = "{\"0\": \"CPU\",\"1\": \"NPU Vector\"}"

	INSTRUMENTATION_TMARK_NAMES(json_str)

	INSTRUMENTATION_END()

if __name__ == "__main__":
	main()