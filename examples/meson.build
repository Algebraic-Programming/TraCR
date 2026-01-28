testSuite = ['examples']

cleanup_script = find_program('bash_scripts/cleanup_script.sh', native: true, required: true)

test_types = ['simple', 'pthread', 'performance_test']

cpp_args_list = [
    ['-DENABLE_TRACR', '-DENABLE_DEBUG'], # Enable TraCR
    []                                    # No instrumentation
]

foreach args : cpp_args_list
    instrumentation_enabled = args.length() > 0 ? true : false

    if instrumentation_enabled
        flag_name = 'tracr_enabled'
    else
        flag_name = 'tracr_disabled'
    endif
    

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

            foreach format : ['paraver', 'perfetto']
                foreach custom_info : [ '', 'custom_both', 'custom_channel_names', 'custom_markerTypes']
                    if custom_info == ''
                        test('tracr_process_' + format + '_' + test_name + '_' + custom_info, tracr_process,
                            workdir: meson.current_build_dir() / '../postprocessing',
                            args : [ output_dir / 'tracr', format],
                            suite : testSuite,
                            is_parallel : false,
                            priority : 0)
                    else
                        if test_type != 'performance_test'
                            custom_info_json = 'bash_scripts/' + custom_info + '.json'
                            config_json = join_paths(meson.current_source_dir(), custom_info_json)
                            test('tracr_process_' + format + '_' + test_name + '_' + custom_info, tracr_process,
                                workdir: meson.current_build_dir() / '../postprocessing',
                                args : [ output_dir / 'tracr', format, config_json],
                                suite : testSuite,
                                is_parallel : false,
                                priority : 0)
                        endif
                    endif
                endforeach
            endforeach
        endif

        test(test_name, exe,
            workdir: output_dir,
            suite : testSuite,
            is_parallel : false,
            priority : 1)
    endforeach
endforeach