#### Testing if TraCR has been installed correctly
testSuite = ['tests']

cpp_args_list = [
    ['-DINSTRUMENTATION_TASKS', '-DINSTRUMENTATION_THREADS', '-DENABLE_DEBUG'], # With task and thread instrumentation
    ['-DINSTRUMENTATION_TASKS', '-DENABLE_DEBUG'],                              # With task instrumentation
    ['-DINSTRUMENTATION_THREADS', '-DENABLE_DEBUG'],                            # With thread instrumentation
    []                                                                          # No instrumentation
]

# TEST 0: Basic ovni test
foreach args : cpp_args_list

  instrumentation_enabled = args.length() > 0 ? true : false

    if args.length() == 3
        flag_name = 'both_instrumentation'

        # TEST 0: Basic ovni test (instrumentation enabled)
        basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)

    elif '-DINSTRUMENTATION_THREADS' in args and '-DINSTRUMENTATION_TASKS' not in args
        flag_name = 'thread_instrumentation'
    elif '-DINSTRUMENTATION_THREADS' not in args and '-DINSTRUMENTATION_TASKS' in args
        flag_name = 'task_instrumentation'
    elif args.length() == 0
        flag_name = 'no_instrumentation'

        # TEST 0: Basic ovni test (instrumentation disabled)
        basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)

    else
        error('RuntimeError: Unknown args: ', args)
    endif
endforeach