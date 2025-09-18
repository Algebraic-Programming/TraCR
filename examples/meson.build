testSuite = ['examples']

cleanup_script = find_program('bash_scripts/cleanup_script.sh', native: true, required: true)

ovniemu_test_script = find_program('bash_scripts/ovniemu_test.sh', native: true, required: true)

create_dir_script = find_program('bash_scripts/create_dirs.py', native: true, required: true)

#### TEST: only thread markers or non

test_types = ['pthread_example', 'thread_markers', 'vmarkers_push_pop', 'vmarkers_set']

cpp_args_list = [
    ['-DENABLE_INSTRUMENTATION', '-DENABLE_DEBUG'], # With thread instrumentation
    []                                               # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    flag_name = instrumentation_enabled ? 'with_instrumentation' : 'no_instrumentation'
    

    foreach test_type : test_types
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

            # Create the output directory before tests
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
    endforeach
endforeach