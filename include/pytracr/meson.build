# pybind11 stuff with the compiler flags

py = import('python').find_installation(pure: false)
pybind11_dep = dependency('pybind11', required: true)

add_project_arguments('-DENABLE_INSTRUMENTATION', language: 'cpp')

py.extension_module('tracr',
    'pytracr.cpp',
    install: true,
    dependencies : [InstrumentationBuildDep, pybind11_dep],
    cpp_args: ['-DENABLE_INSTRUMENTATION'],
)