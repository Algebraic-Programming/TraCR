testSuite = ['examples']

cleanup_script = find_program('bash_scripts/cleanup_script.sh', native: true, required: true)

# create_dir_script = find_program('bash_scripts/create_dirs.py', native: true, required: true)

test_types = ['simple_example', 'pthread_example']

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
                    is_parallel : false)
        endif

        test(test_name, exe,
            workdir: output_dir,
            suite : testSuite,
            is_parallel : false)
    endforeach
endforeach