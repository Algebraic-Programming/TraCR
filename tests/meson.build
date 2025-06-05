#### Testing if TraCR has been installed correctly
testSuite = ['tests']

cpp_args_list = [
    ['-DENABLE_INSTRUMENTATION', '-DENABLE_DEBUG'],  # With thread instrumentation
    []                                               # No instrumentation
]

# TEST 0: Basic ovni test
foreach args : cpp_args_list
  instrumentation_enabled = args.length() > 0 ? true : false

  flag_name = instrumentation_enabled ? 'with_instrumentation' : 'no_instrumentation'

  if instrumentation_enabled
      basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

      test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)
  else
      basic_check_exe = executable('basic_check_' + flag_name, 'basic_check.cpp', dependencies: InstrumentationBuildDep, cpp_args : args)

      test('basic_check_' + flag_name, basic_check_exe, args : [], suite : testSuite)
  endif
endforeach