testSuite = ['ovni']

if buildInstrumentations
    basic_exe = executable('ovni_test', 'basic_ovni.cpp', dependencies:  TaskRBuildDep)
    
    if get_option('buildTests')
        test('basic_ovni', basic_exe, args : [ ], suite: testSuite )
    endif
    
    trace_exe = executable('trace_test', ['trace_test.cpp'] + instrumentation_src, dependencies:  TaskRBuildDep)
    
    if get_option('buildTests')
        test('trace_test', trace_exe, args : [ ], suite: testSuite )
    endif

    thrd_states_exe = executable('thrd_states_test', ['thread_states.cpp'] + instrumentation_src, dependencies:  TaskRBuildDep)
    
    if get_option('buildTests')
        test('thrd_states_test', thrd_states_exe, args : [ ], suite: testSuite )
    endif

    pthreads_example_exe = executable('pthreads_example', ['pthreads_example.cpp'] + instrumentation_src, dependencies:  TaskRBuildDep)
    
    if get_option('buildTests')
        test('pthreads_example', pthreads_example_exe, args : [ ], suite: testSuite )
    endif
endif