testSuite = ['ovni']

cleanup_script = find_program('cleanup_script.sh', native: true, required: true)

ovniemu_test_script = find_program('ovniemu_test.sh', native: true, required: true)

# TODO: allow parallelism if possible.

#### TEST 1: All cases: {tasks & threads tasks, threads, non}

test_types = ['pthread_example']

cpp_args_list = [
    ['-DINSTRUMENTATION_TASKS', '-DINSTRUMENTATION_THREADS', '-DENABLE_DEBUG'], # With task and thread instrumentation
    ['-DINSTRUMENTATION_TASKS', '-DENABLE_DEBUG'],                              # With task instrumentation
    ['-DINSTRUMENTATION_THREADS', '-DENABLE_DEBUG'],                            # With thread instrumentation
    []                                                                          # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    if args.length() == 3
        flag_name = 'both_instrumentation'

        # TEST 0: Basic ovni test (instrumentation disabled)
        basic_ovni_exe = executable('basic_ovni_' + flag_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_ovni_' + flag_name, basic_ovni_exe, args : [], suite : testSuite)

    elif '-DINSTRUMENTATION_THREADS' in args and '-DINSTRUMENTATION_TASKS' not in args
        flag_name = 'thread_instrumentation'
    elif '-DINSTRUMENTATION_THREADS' not in args and '-DINSTRUMENTATION_TASKS' in args
        flag_name = 'task_instrumentation'
    elif args.length() == 0
        flag_name = 'no_instrumentation'

        # TEST 0: Basic ovni test (instrumentation disabled)
        basic_ovni_exe = executable('basic_ovni_' + flag_name, 'basic_ovni.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_ovni_' + flag_name, basic_ovni_exe, args : [], suite : testSuite)

    else
        error('RuntimeError: Unknown args: ', args)
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

#### TEST 2: only thread markers or non

test_types = ['thread_markers', 'markers_vanilla']

cpp_args_list = [
    ['-DINSTRUMENTATION_THREADS', '-DENABLE_DEBUG'], # With thread instrumentation
    []                                               # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    flag_name = instrumentation_enabled ? 'thread_instrumentation' : 'no_instrumentation'

    

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

#### TEST 3: only task markers or non

test_types = ['task_markers']

cpp_args_list = [
    ['-DINSTRUMENTATION_TASKS', '-DENABLE_DEBUG'], # With thread instrumentation
    []                                             # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    flag_name = instrumentation_enabled ? 'task_instrumentation' : 'no_instrumentation'

    

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