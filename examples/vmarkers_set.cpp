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

#include <tracr.hpp>

/**
 * Basic c code to demonstrate TraCR's VMAKERS
 * This example uses the VMARKER SET method
 * you can create a nice gradient plot in Paraver with it
 */
int main(void) {
  // Initialize TraCR
  // This boolean is a check to see if ovni has been initialize by another
  // library (e.g. nOS-V)
  bool externally_init = false;
  INSTRUMENTATION_START(externally_init);

  // use flag == 1 for push/pop and flag == 0 for the set method
  bool flag = 0;
  INSTRUMENTATION_VMARKER_TYPE(flag, "Simple Marker Example");

  const int n = 150;
  for (int i = 1; i <= n; ++i) {
    INSTRUMENTATION_VMARKER_SET(i);
    printf("%d ", i);
  }

  // TraCR finished
  INSTRUMENTATION_END();

  return 0;
}