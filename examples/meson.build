testSuite = ['ovni']

basic_exe = executable('basic_ovni', 'basic_ovni.cpp', dependencies:  InstrumentationBuildDep)

if get_option('buildTests')
    test('basic_ovni', basic_exe, args : [ ], suite: testSuite )
endif

trace_exe = executable('trace_test', ['trace_test.cpp'], dependencies:  InstrumentationBuildDep)

if get_option('buildTests')
    test('trace_test', trace_exe, args : [ ], suite: testSuite )
endif

pthreads_example_exe = executable('pthreads_example', ['pthreads_example.cpp'], dependencies:  InstrumentationBuildDep)

if get_option('buildTests')
    test('pthreads_example', pthreads_example_exe, args : [ ], suite: testSuite )
endif