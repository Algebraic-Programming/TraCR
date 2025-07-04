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

#include <ovni.h>
#include <tracr.hpp>

/*
 * Basic test to see if TraCR (and ovni) is installed correctly
 */
int main(int argc, char **argv) {
  printf("OVNI_TRACEDIR: %s\n", OVNI_TRACEDIR);

  printf("Instrumentation enabled: %d\n", INSTRUMENTATION_ACTIVE);

  ovni_version_check();

  return 0;
}
