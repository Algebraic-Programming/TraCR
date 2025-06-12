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

#include <chrono>
#include <tracr.hpp>

#define mytype float

/**
 * print the square matrices nice
 */
static void print_matrix(mytype *matrix, const size_t N) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      printf("%f ", matrix[i * N + j]);
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * Basic c code to demonstrate TraCR's VMAKERS
 * VMakers (Vanilla Markers) are the type of markers pushing the color ID
 * directly.
 *
 * This example is the same as the thread_markers.cpp but with VMARKERS
 *
 * They are useful if you don't wanna have labels and wanna keep track the
 * number.
 *
 * Still, one can use INSTRUMENTATION_VMARKER_LABEL() if labels are of need.
 * But then you need to remember which color id corresponds to which label.
 */
int main(void) {
  std::chrono::time_point<std::chrono::system_clock> start, end,
      after_label_set;

  start = std::chrono::system_clock::now();

  const size_t N = 4;

  // Initialize TraCR
  // This boolean is a check to see if ovni has been initialize by another
  // library (e.g. nOS-V)
  bool externally_init = false;
  INSTRUMENTATION_START(externally_init);

  // use flag == 1 for push/pop and flag == 0 for the set method
  bool flag = 1;
  INSTRUMENTATION_VMARKER_TYPE(flag, "Simple Marker Example");

  // Each Label creation costs around (~3us)
  // Should be done at the beginning or at the ending of the code
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_LIGHT_GREEN, "Allocate Memory");
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_LAVENDER,
                                "Fill matrices with values");
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_MAROON, "Print all matrices");
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_OLIVE, "MMM");
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_NAVY, "Print solution of matrix A");
  INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_PINK, "Free memory");

  after_label_set = std::chrono::system_clock::now();

  // allocate memory
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_LIGHT_GREEN);
  mytype *A = (mytype *)calloc(1, N * N * sizeof(mytype));
  mytype *B = (mytype *)malloc(N * N * sizeof(mytype));
  mytype *C = (mytype *)malloc(N * N * sizeof(mytype));

  // fill matrices
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_LAVENDER);
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      B[i * N + j] = (mytype)i;
      C[i * N + j] = (mytype)j;
    }
  }
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_LAVENDER);
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_LIGHT_GREEN);

  // print matrices
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_MAROON);
  printf("A:\n");
  print_matrix(A, N);

  printf("B:\n");
  print_matrix(B, N);

  printf("C:\n");
  print_matrix(C, N);
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_MAROON);

  // mmm
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_OLIVE);
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      for (size_t k = 0; k < N; ++k) {
        A[i * N + j] += B[i * N + k] * C[k * N + j];
      }
    }
  }
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_OLIVE);

  // last print
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_NAVY);
  printf("A (after mmm):\n");
  print_matrix(A, N);
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_NAVY);

  // free memory
  INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_PINK);
  free(A);
  free(B);
  free(C);
  INSTRUMENTATION_VMARKER_POP(MARK_COLOR_PINK);

  // TraCR finished
  INSTRUMENTATION_END();

  end = std::chrono::system_clock::now();

  std::chrono::duration<double> total_time = (end - start);

  std::chrono::duration<double> label_set_time = (after_label_set - start);

  std::chrono::duration<double> push_pop_time = (end - after_label_set);

  printf("\n\nTotal time: %f [s]\n", total_time.count());

  printf("Label set time: %f [s]\n", label_set_time.count());

  printf("Push/Pop time: %f [s]\n", push_pop_time.count());

  return 0;
}