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

#include <pybind11/pybind11.h>

#include <tracr/tracr.hpp>

namespace py = pybind11;

static bool instrumentation_active() { return INSTRUMENTATION_ACTIVE; }

/**
 * ovni proc methods
 */
static void instrumentation_start(bool external_init_ = false) {
  INSTRUMENTATION_START(external_init_);
}

static void instrumentation_end_() { INSTRUMENTATION_END(); }

/**
 * ovni thread methods
 */
static void instrumentation_thread_init() { INSTRUMENTATION_THREAD_INIT(); }

static void instrumentation_thread_end_() { INSTRUMENTATION_THREAD_END(); }

/**
 * ovni task marker methods
 */
static void instrumentation_task_init() { INSTRUMENTATION_TASK_INIT(); }

static void instrumentation_tmark_init(size_t flag) {
  INSTRUMENTATION_TMARK_INIT(flag);
}

static size_t instrumentation_tmark_add(size_t value, const char *label) {
  return INSTRUMENTATION_TMARK_ADD(value, label);
}

static size_t instrumentation_tmark_lazy_add(const char *label) {
  return INSTRUMENTATION_TMARK_LAZY_ADD(label);
}

static void instrumentation_tmark_set(uint32_t taskid, size_t idx) {
  INSTRUMENTATION_TMARK_SET(taskid, idx);
}

static void instrumentation_tmark_push(uint32_t taskid, size_t idx) {
  INSTRUMENTATION_TMARK_PUSH(taskid, idx);
}

static void instrumentation_tmark_pop(uint32_t taskid, size_t idx) {
  INSTRUMENTATION_TMARK_POP(taskid, idx);
}

/**
 * ovni thread marker methods
 */
static void instrumentation_mark_init(size_t flag) {
  INSTRUMENTATION_MARK_INIT(flag);
}

static size_t instrumentation_mark_add(size_t value, const char *label) {
  return INSTRUMENTATION_MARK_ADD(value, label);
}

static size_t instrumentation_mark_lazy_add(const char *label) {
  return INSTRUMENTATION_MARK_LAZY_ADD(label);
}

static void instrumentation_mark_set(size_t idx) {
  INSTRUMENTATION_MARK_SET(idx);
}

static void instrumentation_mark_push(size_t idx) {
  INSTRUMENTATION_MARK_PUSH(idx);
}

static void instrumentation_mark_pop(size_t idx) {
  INSTRUMENTATION_MARK_POP(idx);
}

static void instrumentation_mark_reset() { INSTRUMENTATION_MARK_RESET(); }

/**
 * ovni marker methods (vanilla)
 */
static void instrumentation_vmark_type(size_t flag, const char *title) {
  INSTRUMENTATION_VMARK_TYPE(flag, title);
}

static void instrumentation_vmark_label(size_t value, const char *label) {
  INSTRUMENTATION_VMARK_LABEL(value, label);
}

static void instrumentation_vmark_set(size_t value) {
  INSTRUMENTATION_VMARK_SET(value);
}

static void instrumentation_vmark_push(size_t value) {
  INSTRUMENTATION_VMARK_PUSH(value);
}

static void instrumentation_vmark_pop(size_t value) {
  INSTRUMENTATION_VMARK_POP(value);
}

PYBIND11_MODULE(tracr, m) {
  m.doc() = "pybind11 plugin for TraCR";

  m.def("INSTRUMENTATION_ACTIVE", &instrumentation_active, "");

  /**
   * ovni proc methods
   */
  m.def("INSTRUMENTATION_START", &instrumentation_start,
        py::arg("external_init_") = false, "");
  m.def("INSTRUMENTATION_END", &instrumentation_end_, "");

  /**
   * ovni thread methods
   */
  m.def("INSTRUMENTATION_THREAD_INIT", &instrumentation_thread_init, "");
  m.def("INSTRUMENTATION_THREAD_END", &instrumentation_thread_end_, "");

  /**
   * ovni task marker methods
   */
  m.def("INSTRUMENTATION_TASK_INIT", &instrumentation_task_init, "");
  m.def("INSTRUMENTATION_TMARK_INIT", &instrumentation_tmark_init, "");
  m.def("INSTRUMENTATION_TMARK_LAZY_ADD", &instrumentation_tmark_lazy_add, "");
  m.def("INSTRUMENTATION_TMARK_ADD", &instrumentation_tmark_add, "");
  m.def("INSTRUMENTATION_TMARK_SET", &instrumentation_tmark_set, "");
  m.def("INSTRUMENTATION_TMARK_PUSH", &instrumentation_tmark_push, "");
  m.def("INSTRUMENTATION_TMARK_POP", &instrumentation_tmark_pop, "");

  /**
   * ovni thread marker methods
   */
  m.def("INSTRUMENTATION_MARK_INIT", &instrumentation_mark_init, "");
  m.def("INSTRUMENTATION_MARK_LAZY_ADD", &instrumentation_mark_lazy_add, "");
  m.def("INSTRUMENTATION_MARK_ADD", &instrumentation_mark_add, "");
  m.def("INSTRUMENTATION_MARK_SET", &instrumentation_mark_set, "");
  m.def("INSTRUMENTATION_MARK_PUSH", &instrumentation_mark_push, "");
  m.def("INSTRUMENTATION_MARK_POP", &instrumentation_mark_pop, "");
  m.def("INSTRUMENTATION_MARK_RESET", &instrumentation_mark_reset, "");

  /**
   * ovni marker methods (vanilla)
   */
  m.def("INSTRUMENTATION_VMARK_TYPE", &instrumentation_vmark_type, "");
  m.def("INSTRUMENTATION_VMARK_LABEL", &instrumentation_vmark_label, "");
  m.def("INSTRUMENTATION_VMARK_SET", &instrumentation_vmark_set, "");
  m.def("INSTRUMENTATION_VMARK_PUSH", &instrumentation_vmark_push, "");
  m.def("INSTRUMENTATION_VMARK_POP", &instrumentation_vmark_pop, "");

  py::enum_<mark_color>(m, "mark_color", py::arithmetic())
      .value("MARK_COLOR_BLACK", MARK_COLOR_BLACK)
      .value("MARK_COLOR_BLUE", MARK_COLOR_BLUE)
      .value("MARK_COLOR_LIGHT_GRAY", MARK_COLOR_LIGHT_GRAY)
      .value("MARK_COLOR_RED", MARK_COLOR_RED)
      .value("MARK_COLOR_GREEN", MARK_COLOR_GREEN)
      .value("MARK_COLOR_YELLOW", MARK_COLOR_YELLOW)
      .value("MARK_COLOR_ORANGE", MARK_COLOR_ORANGE)
      .value("MARK_COLOR_PURPLE", MARK_COLOR_PURPLE)
      .value("MARK_COLOR_CYAN", MARK_COLOR_CYAN)
      .value("MARK_COLOR_MAGENTA", MARK_COLOR_MAGENTA)
      .value("MARK_COLOR_LIGHT_GREEN", MARK_COLOR_LIGHT_GREEN)
      .value("MARK_COLOR_PINK", MARK_COLOR_PINK)
      .value("MARK_COLOR_TEAL", MARK_COLOR_TEAL)
      .value("MARK_COLOR_GRAY", MARK_COLOR_GRAY)
      .value("MARK_COLOR_LAVENDER", MARK_COLOR_LAVENDER)
      .value("MARK_COLOR_BROWN", MARK_COLOR_BROWN)
      .value("MARK_COLOR_LIGHT_YELLOW", MARK_COLOR_LIGHT_YELLOW)
      .value("MARK_COLOR_MAROON", MARK_COLOR_MAROON)
      .value("MARK_COLOR_MINT", MARK_COLOR_MINT)
      .value("MARK_COLOR_OLIVE", MARK_COLOR_OLIVE)
      .value("MARK_COLOR_PEACH", MARK_COLOR_PEACH)
      .value("MARK_COLOR_NAVY", MARK_COLOR_NAVY)
      .value("MARK_COLOR_BRIGHT_BLUE", MARK_COLOR_BRIGHT_BLUE)
      .export_values();
}
