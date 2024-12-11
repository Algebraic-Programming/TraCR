testSuite = ['ovni']

if get_option('enableInstrumentations')
    cpp_args_list = [
    []                                            # instrumentation already enabled
    ]
else
    cpp_args_list = [
    ['-DUSE_INSTRUMENTATION', '-DENABLE_DEBUG'],  # With instrumentation
    []                                            # Without instrumentation
    ]
endif

# TODO: allow parallelism if possible.

test_types = ['pthreads_task_markers', 'pthreads_thread_markers', 'thread_markers', 'task_markers', 'markers_vanilla']
foreach args : cpp_args_list
    instrumentation_enabled = get_option('enableInstrumentations') or args == ['-DUSE_INSTRUMENTATION', '-DENABLE_DEBUG'] ? true : false

    flag_name = instrumentation_enabled ? 'with_instrumentation' : 'no_instrumentation'

    if instrumentation_enabled
        cleanup_script = find_program('cleanup_script.sh', native: true, required: true)

        ovniemu_test_script = find_program('ovniemu_test.sh', native: true, required: true)
    endif


    # TEST 1: Basic ovni test
    basic_ovni_exe = executable('basic_ovni_' + flag_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

    test('basic_ovni_' + flag_name, basic_ovni_exe, args : [], suite : testSuite)

    foreach test_type : test_types
        test_name = test_type + '_' + flag_name
        exe = executable(test_name, test_type + '.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        output_dir = meson.build_root() + '/examples/' + test_name + '.p'

        if instrumentation_enabled
            test('cleanup_script_' + test_name, cleanup_script, 
                    workdir: output_dir,
                    suite : testSuite,
                    is_parallel : false,
                    priority : 2)
        endif

        test(test_name, exe,
             workdir: output_dir,
             suite : testSuite,
             is_parallel : false,
             priority : 1)

        if instrumentation_enabled
            test('ovniemu_test_' + test_name, ovniemu_test_script, 
                 workdir: output_dir,
                 suite : testSuite,
                 is_parallel : false,
                 priority : 0)
        endif
    endforeach
endforeach