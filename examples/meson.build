testSuite = ['ovni']

cleanup_script = find_program('cleanup_script.sh', native: true, required: true)

ovniemu_test_script = find_program('ovniemu_test.sh', native: true, required: true)

# TODO: allow parallelism if possible.

#### TEST: only thread markers or non

test_types = ['pthread_example', 'thread_markers', 'markers_vanilla']

cpp_args_list = [
    ['-DENABLE_INSTRUMENTATION', '-DENABLE_DEBUG'], # With thread instrumentation
    []                                               # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    flag_name = instrumentation_enabled ? 'with_instrumentation' : 'no_instrumentation'

    # TEST 0: Basic ovni test
    if instrumentation_enabled
        basic_ovni_exe = executable('basic_ovni_' + flag_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_ovni_' + flag_name, basic_ovni_exe, args : [], suite : testSuite)
    else
        basic_ovni_exe = executable('basic_ovni_' + flag_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_ovni_' + flag_name, basic_ovni_exe, args : [], suite : testSuite)
    endif

    

    foreach test_type : test_types
        test_name = test_type + '_' + flag_name
        exe = executable(test_name, test_type + '.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        output_dir = exe.path() + '.p'

        if instrumentation_enabled
            test('cleanup_script_' + test_name, cleanup_script, 
                    workdir: output_dir,
                    suite : testSuite,
                    is_parallel : false,
                    priority : 2)

            test('ovniemu_test_' + test_name, ovniemu_test_script, 
                 workdir: output_dir,
                 suite : testSuite,
                 is_parallel : false,
                 priority : 0)
        endif

        test(test_name, exe,
             workdir: output_dir,
             suite : testSuite,
             is_parallel : false,
             priority : 1)
            
    endforeach
endforeach