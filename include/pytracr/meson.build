# pybind11 stuff with the compiler flags

py = import('python').find_installation('/usr/bin/python3', pure: false)
pybind11_dep = dependency('pybind11', required: true)

py.extension_module('tracr',
    'pytracr.cpp',
    install: true,
    dependencies : [InstrumentationBuildDep, pybind11_dep],
    cpp_args: ['-DINSTRUMENTATION_TASKS', '-DINSTRUMENTATION_THREADS',],
)