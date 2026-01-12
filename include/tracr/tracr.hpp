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

/**
 * @file tracr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Andrés Baumann
 * @date 09/01/2026
 */

#pragma once

#include "tracr_core.hpp"

/**
 * Marker colors values of the default Paraver color palette
 * 
 * NOTE: Some colors might be wrong when using Paraver in light mode instead of night mode
 */
enum mark_color : uint16_t {
  MARK_COLOR_NONE = 0,
  MARK_COLOR_BLUE,
  MARK_COLOR_LIGHT_GRAY,
  MARK_COLOR_RED,
  MARK_COLOR_GREEN,
  MARK_COLOR_YELLOW,
  MARK_COLOR_ORANGE,
  MARK_COLOR_PURPLE,
  MARK_COLOR_CYAN,
  MARK_COLOR_MAGENTA,
  MARK_COLOR_LIGHT_GREEN,
  MARK_COLOR_PINK,
  MARK_COLOR_TEAL,
  MARK_COLOR_GRAY,
  MARK_COLOR_LAVENDER,
  MARK_COLOR_BROWN,
  MARK_COLOR_LIGHT_YELLOW,
  MARK_COLOR_MAROON,
  MARK_COLOR_MINT,
  MARK_COLOR_OLIVE,
  MARK_COLOR_PEACH,
  MARK_COLOR_NAVY,
  MARK_COLOR_BRIGHT_BLUE
};

/**
 * Using this flag will enable all the instrumentations of TraCR. Otherwise pure
 * void functions.
 */
#ifdef ENABLE_TRACR

/**
 * This boolean is needed if something other than TraCR has to be called.
 */
#define INSTRUMENTATION_ACTIVE true

/**
 * Proc methods
 */
#define INSTRUMENTATION_START(path) instrumentation_start(path)

#define INSTRUMENTATION_END() instrumentation_end()

/**
 * Thread methods
 */
#define INSTRUMENTATION_THREAD_INIT() instrumentation_thread_init()

#define INSTRUMENTATION_THREAD_FINALIZE() instrumentation_thread_finalize()

/**
 * Marker methods
 */
#define INSTRUMENTATION_MARK_ADD(colorId, label) instrumentation_mark_add(colorId, label)

#define INSTRUMENTATION_MARK_LAZY_ADD(label) instrumentation_mark_lazy_add(label)

#define INSTRUMENTATION_MARK_SET(channelId, eventId, extraId) instrumentation_mark_set(channelId, eventId, extraId)

#define INSTRUMENTATION_MARK_RESET(channelId) instrumentation_mark_reset(channelId)

#define INSTRUMENTATION_ADD_CHANNEL_NAMES(channel_names)  tracrProc->addCustomChannelNames(channel_names)

#define INSTRUMENTATION_ADD_NUM_CHANNELS(num_channels) tracrProc->addNumberOfChannels(num_channels)

/**
 * Other methods
 */
#define INSTRUMENTATION_ON() instrumentation_on()

#define INSTRUMENTATION_OFF() instrumentation_off()

#else /* ENABLE_TRACR */

/**
 * This boolean is needed if something other than TraCR has to be called.
 */
#define INSTRUMENTATION_ACTIVE false

/**
 * Main proc methods
 */
#define INSTRUMENTATION_START(path) (void)(path)
  
#define INSTRUMENTATION_END()


/**
 * Thread methods
 */
#define INSTRUMENTATION_THREAD_INIT()

#define INSTRUMENTATION_THREAD_FINALIZE()

/**
 * Marker methods
 */
#define INSTRUMENTATION_MARK_ADD(colorId, label) 0; (void)(label)

#define INSTRUMENTATION_MARK_LAZY_ADD(label) 0

#define INSTRUMENTATION_MARK_SET(channelId, eventId, extraId) (void)(channelId); (void)(eventId); (void)(extraId)

#define INSTRUMENTATION_MARK_RESET(channelId) (void)(channelId)

#define INSTRUMENTATION_ADD_CHANNEL_NAMES(channel_names) (void)(channel_names)

#define INSTRUMENTATION_ADD_NUM_CHANNELS(num_channels) (void)(num_channels)

/**
 * Runtime-level enabling/disabling methods (NOT YET IMPLEMENTED)
 */
#define INSTRUMENTATION_ON()

#define INSTRUMENTATION_OFF()

#endif /* ENABLE_TRACR */
