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
#include <tracr/tracr.hpp>

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
 * This example is the same as the thread_markers.cpp but with VMARKS
 *
 * They are useful if you don't wanna have labels and wanna keep track the
 * number.
 *
 * Still, one can use INSTRUMENTATION_VMARK_LABEL() if labels are of need.
 * But then you need to remember which color id corresponds to which label.
 */
int main(void) {
  std::chrono::time_point<std::chrono::system_clock> start, end,
      after_label_set;

  start = std::chrono::system_clock::now();

  const size_t N = 4;

  // Initialize TraCR
  INSTRUMENTATION_START();

  // use flag == 1 for push/pop and flag == 0 for the set method
  bool flag = 1;
  int32_t type = 0;
  INSTRUMENTATION_VMARK_INIT(type, flag, "Simple Push/Pop Marker Example");

  // Each Label creation costs around (~3us)
  // Should be done at the beginning or at the ending of the code
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_LIGHT_GREEN, "Allocate Memory");
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_LAVENDER,
                              "Fill matrices with values");
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_MAROON, "Print all matrices");
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_OLIVE, "MMM");
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_NAVY,
                              "Print solution of matrix A");
  INSTRUMENTATION_VMARK_LABEL(type, MARK_COLOR_PINK, "Free memory");

  // now the other marker type (this is a test)
  flag = 0;
  type = 1;
  INSTRUMENTATION_VMARK_INIT(type, flag,
                             "Simple Set Marker Example (other type)");

  // Each Label creation costs around (~3us)
  // Should be done at the beginning or at the ending of the code
  INSTRUMENTATION_VMARK_LABEL(type, 23, "Hello1");
  INSTRUMENTATION_VMARK_LABEL(type, 24, "Hello1");
  INSTRUMENTATION_VMARK_LABEL(type, 25, "Hello2");
  INSTRUMENTATION_VMARK_LABEL(type, 26, "Hello3");
  INSTRUMENTATION_VMARK_LABEL(type, 27, "Hello4");
  INSTRUMENTATION_VMARK_LABEL(type, 28, "Hello5");

  after_label_set = std::chrono::system_clock::now();

  // allocate memory
  INSTRUMENTATION_VMARK_SET(1, 23);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_LIGHT_GREEN);
  mytype *A = (mytype *)calloc(1, N * N * sizeof(mytype));
  mytype *B = (mytype *)malloc(N * N * sizeof(mytype));
  mytype *C = (mytype *)malloc(N * N * sizeof(mytype));

  // fill matrices
  INSTRUMENTATION_VMARK_SET(1, 24);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_LAVENDER);
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      B[i * N + j] = (mytype)i;
      C[i * N + j] = (mytype)j;
    }
  }
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_LAVENDER);
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_LIGHT_GREEN);

  // print matrices
  INSTRUMENTATION_VMARK_SET(1, 25);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_MAROON);
  printf("A:\n");
  print_matrix(A, N);

  printf("B:\n");
  print_matrix(B, N);

  printf("C:\n");
  print_matrix(C, N);
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_MAROON);

  // mmm
  INSTRUMENTATION_VMARK_SET(1, 26);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_OLIVE);
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      for (size_t k = 0; k < N; ++k) {
        A[i * N + j] += B[i * N + k] * C[k * N + j];
      }
    }
  }
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_OLIVE);

  // last print
  INSTRUMENTATION_VMARK_SET(1, 27);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_NAVY);
  printf("A (after mmm):\n");
  print_matrix(A, N);
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_NAVY);

  // free memory
  INSTRUMENTATION_VMARK_SET(1, 28);
  INSTRUMENTATION_VMARK_PUSH(0, MARK_COLOR_PINK);
  free(A);
  free(B);
  free(C);
  INSTRUMENTATION_VMARK_POP(0, MARK_COLOR_PINK);

  INSTRUMENTATION_MARK_RESET(1);

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