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
 This is a basic example of the thread markers of TraCR
"""
def main():
  t_start = time.time()

  N = 4

  # Initialize TraCR
  INSTRUMENTATION_START()

  # 0 == Set and 1 == Push/Pop
  INSTRUMENTATION_MARK_INIT(1)

  # Each INSTRUMENTATION_MARK_ADD costs around (~3us)
  # Should be done at the beginning or at the ending of the code
  alloc_mem_label_id = INSTRUMENTATION_MARK_ADD(mark_color.MARK_COLOR_TEAL, "Allocate Memory")
  fill_mat_label_id = INSTRUMENTATION_MARK_ADD(mark_color.MARK_COLOR_LAVENDER, "Fill matrices with values")
  prt_mat_label_id = INSTRUMENTATION_MARK_ADD(mark_color.MARK_COLOR_RED, "Print all matrices")
  mmm_label_id = INSTRUMENTATION_MARK_ADD(mark_color.MARK_COLOR_PEACH, "MMM")
  prt_A_label_id = INSTRUMENTATION_MARK_ADD(mark_color.MARK_COLOR_LIGHT_GRAY, "Print solution of matrix A")

  t_after_label_set = time.time()

  # allocate memory
  INSTRUMENTATION_MARK_PUSH(alloc_mem_label_id)
  A = np.empty((N,N))
  B = np.empty((N,N))
  C = np.empty((N,N))

  # fill matrices
  INSTRUMENTATION_MARK_PUSH(fill_mat_label_id)
  for i in range(N):
    for j in range(N):
      B[i,j] = i
      C[i,j] = j
  INSTRUMENTATION_MARK_POP(fill_mat_label_id)
  INSTRUMENTATION_MARK_POP(alloc_mem_label_id)

  # print matrices
  INSTRUMENTATION_MARK_PUSH(prt_mat_label_id)
  print(f"A: {A}")
  print(f"B: {B}")
  print(f"C: {C}")
  INSTRUMENTATION_MARK_POP(prt_mat_label_id)

  # mmm
  INSTRUMENTATION_MARK_PUSH(mmm_label_id)
  A = B @ C
  INSTRUMENTATION_MARK_POP(mmm_label_id)

  # last print
  INSTRUMENTATION_MARK_PUSH(prt_A_label_id)
  print(f"A: {A}")
  INSTRUMENTATION_MARK_POP(prt_A_label_id)

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