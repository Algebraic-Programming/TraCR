testSuite = ['ovni']

if get_option('enableInstrumentations')
    cpp_args_list = [
    []                          # instrumentation already enabled
    ]
else
    cpp_args_list = [
    ['-DUSE_INSTRUMENTATION'],  # With instrumentation
    []                          # Without instrumentation
    ]
endif

foreach args : cpp_args_list
    test_name = get_option('enableInstrumentations') or args == ['-DUSE_INSTRUMENTATION'] ? 'with_instrumentation' : 'no_instrumentation'


    # TEST 1: Basic ovni test
    basic_ovni_exe = executable('basic_ovni_' + test_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

    test('basic_ovni_' + test_name, basic_ovni_exe, args : [], suite : testSuite)


    # TEST 2: trace test
    trace_test_exe = executable('trace_test_' + test_name, 'trace_test.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

    test_output_dir = 'trace_test_output_' + test_name
    test('trace_test_' + test_name, trace_test_exe,
        #  workdir: meson.build_root(),
         suite : testSuite)

    if get_option('enableInstrumentations') or args == ['-DUSE_INSTRUMENTATION']
        # TODO: do ovniemu ovni/ here
    endif


    # TEST 3: tasking level tracing test with pthreads
    tasking_example_exe = executable('tasking_example_' + test_name, 'tasking_example.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

    test_output_dir = 'tasking_example_output_' + test_name
    test('tasking_example_' + test_name, tasking_example_exe,
        #  workdir: meson.build_root(),
         suite : testSuite)

    if get_option('enableInstrumentations') or args == ['-DUSE_INSTRUMENTATION']
        # TODO: do ovniemu ovni/ here
    endif
endforeach