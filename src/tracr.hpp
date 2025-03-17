/*
 * Copyright Huawei Technologies Switzerland AG
 * All rights reserved.
 */

/**
 * @file tracr.hpp
 * @brief instrumentation calls inside #define functionalities
 * @author Noah Baumann
 * @date 17/12/2024
 */

 #pragma once

 #include <atomic>
 #include <sys/sysinfo.h>    // get_nprocs()
 #include <ovni.h>
 #include "base_instr.hpp"
 
 
 /**
  * Marker colors values of the default Paraver color palette
  * NOTE: Black would be 0 but in ovni it is illegal!
  */
 enum mark_color : int64_t {
     MARK_COLOR_BLUE = 1,
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
  * 
  */
 #ifdef ENABLE_INSTRUMENTATION
 
     // debug printing method. Can be enabled with the ENABLE_DEBUG flag included.
     #ifdef ENABLE_DEBUG
         #define debug_print(fmt, ...) \
             printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
     #else
         #define debug_print(fmt, ...)
     #endif
 
     // keep track of the main thread as this one has to be free'd when instr_end is called
     extern pid_t main_TID;
 
     // A flag to check if something else has initialized ovni (like nosv). If so, TraCR with not init/end proc.
     extern bool external_init;
 
     // this boolean is needed if something other than TraCR has to be called.
     #define INSTRUMENTATION_ACTIVE true    
 
     // ovni proc methods
     #define INSTRUMENTATION_START()                                     \
         debug_print("instr_start (TID: %d)", get_tid());                \
         main_TID = get_tid();                                           \
         external_init = ovni_proc_isready();                            \
         if(!external_init) {                                            \
             instrumentation_init_proc(sched_getcpu(), get_nprocs());    \
         }
     
     #define INSTRUMENTATION_END()                                               \
         debug_print("instr_end (TID: %d)", get_tid());                          \
         if(!external_init) {                                                    \
             instrumentation_end();                                              \
         }
 
     // ovni thread methods
     #define INSTRUMENTATION_THREAD_INIT()                                           \
         debug_print("instr_thread_init with isready: %d (TID: %d)", ovni_thread_isready(), get_tid()); \
         if(!ovni_thread_isready()) {                                                \
             instrumentation_init_thread();                                          \
         }
 
     #define INSTRUMENTATION_THREAD_END()                                            \
         debug_print("instr_thread_end with isready: %d (TID: %d)", ovni_thread_isready(), get_tid());  \
         if(ovni_thread_isready() && !(main_TID == get_tid())) {                                                 \
             instrumentation_thread_end();                                           \
             ovni_thread_free();                                                     \
         }
         
     // ovni thread marker methods
     extern ThreadMarkerMap thread_marker_map;
 
     #define INSTRUMENTATION_THREAD_MARK_INIT(flag)  \
         debug_print("instr_marker_init (TID: %d)", get_tid());   \
         ovni_mark_type(0, flag, "TraCR Thread Markers")
 
     #define INSTRUMENTATION_THREAD_MARK_ADD(labelid, label)  \
         thread_marker_map.add(labelid, label);                    \
         debug_print("instr_marker_add (TID: %d)", get_tid())   \
 
     #define INSTRUMENTATION_THREAD_MARK_SET(idx)  \
         debug_print("instr_marker_set idx: %ld (TID: %d)", idx, get_tid());   \
         thread_marker_map.set(idx);
 
     #define INSTRUMENTATION_THREAD_MARK_PUSH(idx)  \
         debug_print("instr_marker_push (TID: %d)", get_tid());   \
         thread_marker_map.push(idx);
 
     #define INSTRUMENTATION_THREAD_MARK_POP(idx)  \
         debug_print("instr_marker_pop (TID: %d)", get_tid());   \
         thread_marker_map.pop(idx);
 
     // ovni marker methods (vanilla) (only used for performance comparisons)
     #define INSTRUMENTATION_VMARKER_TYPE(flag, title)    \
         ovni_mark_type(0, flag, title)
 
     #define INSTRUMENTATION_VMARKER_LABEL(value, label)  \
         ovni_mark_label(0, value, label)
 
     #define INSTRUMENTATION_VMARKER_SET(value)   \
         debug_print("instr_marker_set (TID: %d)", get_tid());   \
         ovni_mark_set(0, value)
     
     #define INSTRUMENTATION_VMARKER_PUSH(value)  \
         debug_print("instr_marker_push (TID: %d)", get_tid());   \
         ovni_mark_push(0, value)
 
     #define INSTRUMENTATION_VMARKER_POP(value)   \
         debug_print("instr_marker_pop (TID: %d)", get_tid());   \
         ovni_mark_pop(0, value)

 #else   /* No instrumentation (void) */
 
     #define INSTRUMENTATION_ACTIVE false
 
     // ovni proc methods
     #define INSTRUMENTATION_START()
 
     #define INSTRUMENTATION_END()
 
     // ovni thread methods
     #define INSTRUMENTATION_THREAD_INIT()
 
     #define INSTRUMENTATION_THREAD_END()
     
     // ovni thread marker methods
     #define INSTRUMENTATION_THREAD_MARK_INIT(flag) (void)(flag)
     
     #define INSTRUMENTATION_THREAD_MARK_ADD(value, label) -1; (void)(value); (void)(label)
     
     #define INSTRUMENTATION_THREAD_MARK_SET(idx) (void)(idx)
     
     #define INSTRUMENTATION_THREAD_MARK_PUSH(idx) (void)(idx)
     
     #define INSTRUMENTATION_THREAD_MARK_POP(idx) (void)(idx)
     
     // ovni marker methods (vanilla) (only used for performance comparisons)
     #define INSTRUMENTATION_VMARKER_TYPE(flag, title) (void)(flag); (void)(title)
     
     #define INSTRUMENTATION_VMARKER_LABEL(value, label) (void)(value); (void)(label)
     
     #define INSTRUMENTATION_VMARKER_SET(value) (void)(value)
     
     #define INSTRUMENTATION_VMARKER_PUSH(value) (void)(value)
     
     #define INSTRUMENTATION_VMARKER_POP(value) (void)(value)
 #endif
 