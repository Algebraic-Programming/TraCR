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
  INSTRUMENTATION_START();

  /**
   * 0 == Set and 1 == Push/Pop
   */
  INSTRUMENTATION_VTMARK_INIT(0);

  INSTRUMENTATION_VTMARK_ADD(MARK_COLOR_GRAY, "task initialized");
  INSTRUMENTATION_VTMARK_ADD(MARK_COLOR_LIGHT_GREEN, "task executed");
  INSTRUMENTATION_VTMARK_ADD(MARK_COLOR_LIGHT_GRAY, "task suspended");
  INSTRUMENTATION_VTMARK_ADD(MARK_COLOR_BLUE, "task finished");

  for (uint32_t taskid = 0; taskid < ntasks; taskid++) {
    INSTRUMENTATION_TASK_INIT();

    INSTRUMENTATION_VTMARK_SET(taskid, MARK_COLOR_GRAY);

    printf("task(%d) initialized\n", taskid);

    // same value push (should not fail!)
    INSTRUMENTATION_VTMARK_SET(taskid, MARK_COLOR_GRAY);

    printf("task(%d) initialized\n", taskid);

    INSTRUMENTATION_VTMARK_SET(taskid, MARK_COLOR_LIGHT_GREEN);

    printf("task(%d) executed\n", taskid);

    INSTRUMENTATION_VTMARK_SET(taskid, MARK_COLOR_LIGHT_GRAY);

    printf("task(%d) suspended\n", taskid);

    INSTRUMENTATION_VTMARK_SET(taskid, MARK_COLOR_BLUE);

    printf("task(%d) finishing\n", taskid);

    INSTRUMENTATION_TMARK_RESET(taskid);
  }

  const char *json_str = "{\"0\": \"CPU\",\"1\": \"NPU Vector\"}";

  INSTRUMENTATION_TMARK_NAMES(json_str);

  INSTRUMENTATION_END();

  return 0;
}
