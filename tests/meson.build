#### Testing if TraCR has been installed correctly
testSuite = ['tests']

cpp_args_list = [
    ['-DENABLE_TRACR', '-DENABLE_DEBUG'], # Enable TraCR
    []                                    # No instrumentation
]

# TEST 0: Basic ovni test
foreach args : cpp_args_list

  instrumentation_enabled = args.length() > 0 ? true : false

    if instrumentation_enabled
        flag_name = 'both_instrumentation'

        # TEST 0: Basic ovni test (instrumentation enabled)
        basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)
    else
        flag_name = 'no_instrumentation'

        # TEST 0: Basic ovni test (instrumentation disabled)
        basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

        test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)
    endif
endforeach