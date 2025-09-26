testSuite = ['examples']

cleanup_script = find_program('bash_scripts/cleanup_script.sh', native: true, required: true)

ovniemu_test_script = find_program('bash_scripts/ovniemu_test.sh', native: true, required: true)

create_dir_script = find_program('bash_scripts/create_dirs.py', native: true, required: true)

#### TEST: only thread markers or non

test_types = ['pthread_example', 'thread_markers', 'vmarkers_push_pop', 'vmarkers_set', 'task_markers']

valid_flags = {
  'pthread_example'   : ['both_instrumentation', 'thread_instrumentation', 'task_instrumentation', 'no_instrumentation'],
  'thread_markers'    : ['thread_instrumentation', 'no_instrumentation'],
  'vmarkers_push_pop' : ['thread_instrumentation', 'no_instrumentation'],
  'vmarkers_set'      : ['thread_instrumentation', 'no_instrumentation'],
  'task_markers'      : ['task_instrumentation', 'no_instrumentation'],
}

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
    elif '-DINSTRUMENTATION_THREADS' in args and '-DINSTRUMENTATION_TASKS' not in args
        flag_name = 'thread_instrumentation'
    elif '-DINSTRUMENTATION_THREADS' not in args and '-DINSTRUMENTATION_TASKS' in args
        flag_name = 'task_instrumentation'
    elif args.length() == 0
        flag_name = 'no_instrumentation'
    else
        error('RuntimeError: Unknown args: ', args)
    endif
    

    foreach test_type : test_types
        if flag_name in valid_flags.get(test_type, '')

            test_name = test_type + '_' + flag_name
            exe = executable(test_name, 'tracr/' + test_type + '.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

            output_dir = exe.path() + '.p'

            if instrumentation_enabled
                test('cleanup_script_' + test_name, cleanup_script, 
                        workdir: output_dir,
                        suite : testSuite,
                        is_parallel : false,
                        priority : 2)

                test('ovniemu_' + test_name, ovniemu_test_script, 
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

            if get_option('buildPyTraCR') and instrumentation_enabled and (test_type != 'pthread_example')

                test_name = 'pyTraCR_' + test_name
                workdir = meson.project_build_root() + '/examples/' + test_name + '_out/'

                # Create the output directory before running the tests
                create_dirs = custom_target('create_pytracr_dirs' + test_name,
                    output: test_name + '_out.stamp',
                    command: [create_dir_script, workdir, '&&', 'touch', '@OUTPUT@'],
                    build_by_default: true
                )

                test(test_name,
                    py,
                    args: [meson.current_source_dir() + '/pytracr/' + test_type + '.py'],
                    env: ['PYTHONPATH=' + meson.project_build_root() + '/include/pytracr/'],
                    suite: testSuite,
                    workdir: workdir,
                    is_parallel: false,
                    priority: 1,
                    depends: [create_dirs]
                )

                test('cleanup_script_' + test_name, cleanup_script,
                    workdir: workdir,
                    suite: testSuite,
                    is_parallel: false,
                    priority: 2
                )

                test('ovniemu_' + test_name, ovniemu_test_script,
                    workdir: workdir,
                    suite: testSuite,
                    is_parallel: false,
                    priority: 0,
                    depends: [create_dirs]
                )

            endif
        endif
    endforeach
endforeach