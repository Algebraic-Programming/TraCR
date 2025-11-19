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

import time
import numpy as np
from tracr import *

"""
 Basic c code to demonstrate TraCR's VMAKERS
 VMakers (Vanilla Markers) are the type of markers pushing the color ID
 directly.
 
 This example is the same as the thread_markers.cpp but with VMARKS
 They are useful if you don't wanna have labels and wanna keep track the
 number.
 
 Still, one can use INSTRUMENTATION_VMARK_LABEL() if labels are of need.
 But then you need to remember which color id corresponds to which label.
"""
def main():
  t_start = time.time()

  N = 4

  # Initialize TraCR
  INSTRUMENTATION_START()

  # 0 == Set and 1 == Push/Pop
  flag = 1
  INSTRUMENTATION_VMARK_INIT(flag, "Simple Marker Example")

  # Each INSTRUMENTATION_MARK_ADD costs around (~3us)
  # Should be done at the beginning or at the ending of the code
  INSTRUMENTATION_VMARK_LABEL(MARK_COLOR_LIGHT_GREEN, "Allocate Memory")
  INSTRUMENTATION_VMARK_LABEL(MARK_COLOR_LAVENDER,
                                "Fill matrices with values")
  INSTRUMENTATION_VMARK_LABEL(MARK_COLOR_MAROON, "Print all matrices")
  INSTRUMENTATION_VMARK_LABEL(MARK_COLOR_OLIVE, "MMM")
  INSTRUMENTATION_VMARK_LABEL(MARK_COLOR_NAVY, "Print solution of matrix A")

  t_after_label_set = time.time()

  # allocate memory
  INSTRUMENTATION_VMARK_PUSH(MARK_COLOR_LIGHT_GREEN)
  A = np.empty((N,N))
  B = np.empty((N,N))
  C = np.empty((N,N))

  # fill matrices
  INSTRUMENTATION_VMARK_PUSH(MARK_COLOR_LAVENDER)
  for i in range(N):
    for j in range(N):
      B[i, j] = i
      C[i, j] = j
  INSTRUMENTATION_VMARK_POP(MARK_COLOR_LAVENDER)
  INSTRUMENTATION_VMARK_POP(MARK_COLOR_LIGHT_GREEN)

  # print matrices
  INSTRUMENTATION_VMARK_PUSH(MARK_COLOR_MAROON)
  print(f"A: {A}")
  print(f"B: {B}")
  print(f"C: {C}")
  INSTRUMENTATION_VMARK_POP(MARK_COLOR_MAROON)

  # mmm
  INSTRUMENTATION_VMARK_PUSH(MARK_COLOR_OLIVE)
  A = B @ C
  INSTRUMENTATION_VMARK_POP(MARK_COLOR_OLIVE)

  # last print
  INSTRUMENTATION_VMARK_PUSH(MARK_COLOR_NAVY)
  print(f"A: {A}")
  INSTRUMENTATION_VMARK_POP(MARK_COLOR_NAVY)

  # TraCR finished
  INSTRUMENTATION_END()

  t_end = time.time()

  total_time = t_end - t_start

  label_set_time = t_after_label_set - t_start

  push_pop_time = t_end - t_after_label_set

  print(f"\nTotal time: {total_time}s")

  print(f"Label set time: {label_set_time}s")

  print(f"Push/Pop time: {push_pop_time}s")

if __name__ == "__main__":
    main()