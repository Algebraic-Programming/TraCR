/*
 *   Copyright 2025 Huawei Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tracr/tracr.hpp>

int main(void) {
  const uint32_t ntasks = 2;

  // Initialize TraCR
  // This boolean is a check to see if ovni has been initialize by another
  // library (e.g. nOS-V)
  bool externally_init = false;
  INSTRUMENTATION_START(externally_init);

  /**
   * 0 == Set and 1 == Push/Pop
   */
  INSTRUMENTATION_TMARK_INIT(0);

  const size_t task_init_idx =
      INSTRUMENTATION_TMARK_ADD(MARK_COLOR_GRAY, "task initialized");
  const size_t task_exec_idx =
      INSTRUMENTATION_TMARK_ADD(MARK_COLOR_LIGHT_GREEN, "task executed");
  const size_t task_susp_idx =
      INSTRUMENTATION_TMARK_ADD(MARK_COLOR_LIGHT_GRAY, "task suspended");
  const size_t task_fini_idx =
      INSTRUMENTATION_TMARK_ADD(MARK_COLOR_BLACK, "task finished");

  for (uint32_t taskid = 0; taskid < ntasks; taskid++) {

    INSTRUMENTATION_TASK_INIT();

    INSTRUMENTATION_TMARK_SET(taskid, task_init_idx);

    printf("task(%d) initialized\n", taskid);

    // same value push (should not fail!)
    INSTRUMENTATION_TMARK_SET(taskid, task_init_idx);

    printf("task(%d) initialized\n", taskid);

    INSTRUMENTATION_TMARK_SET(taskid, task_exec_idx);

    printf("task(%d) executed\n", taskid);

    INSTRUMENTATION_TMARK_SET(taskid, task_susp_idx);

    printf("task(%d) suspended\n", taskid);

    INSTRUMENTATION_TMARK_SET(taskid, task_fini_idx);

    printf("task(%d) finishing\n", taskid);
  }

  INSTRUMENTATION_END();

  return 0;
}
