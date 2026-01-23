/*
 *   Copyright 2026 Huawei Technologies Co., Ltd.
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

#include <chrono>
#include <nlohmann/json.hpp>
#include <tracr/tracr.hpp>

/**
 * This is a basic example of the thread markers of TraCR
 */
int main(void) {
  // Initialize TraCR
  INSTRUMENTATION_START("");

  // performance test
  auto perf_test_start = std::chrono::system_clock::now();
  const int n_sets = 1e6;
  for (int i = 0; i < n_sets; ++i) {
    INSTRUMENTATION_MARK_SET(0, i % 128, 0);
  }
  auto perf_test_stop = std::chrono::system_clock::now();

  std::chrono::duration<double> perf_time = (perf_test_stop - perf_test_start);

  printf("Setting %d markers costs: %f[ms] and on average: %f[ns]\n", n_sets,
         perf_time.count() * 1e6, perf_time.count() * 1e9 / double(n_sets));

  // TraCR finished
  INSTRUMENTATION_END();

  return 0;
}