project('TraCR', [ 'cpp', 'c'],
  subproject_dir : 'extern',
  default_options : [
  'cpp_std=c++11',
  'buildtype=release',
  ]
)

####### Storage for Instrumentation dependencies



# Warning handling option
warningAsErrorFlags=[]
if get_option('compileWarningsAsErrors') == true
  warningAsErrorFlags=[ '-Werror' ]
endif

InstrumentationBuildCppArgs = [
 '-Wfatal-errors',
  warningAsErrorFlags,
 ]

 InstrumentationBuildIncludes = include_directories([
   'include'
  ])
  

####### Build ovni

subdir('include/tracr')

####### Collect the dependencies

InstrumentationBuildDep = declare_dependency(
    compile_args: InstrumentationBuildCppArgs,
    include_directories: InstrumentationBuildIncludes,
    sources: ['include/tracr/tracr.cpp'],
    dependencies: ovniDep
    )

####### Build PyTaskR
if get_option('buildPyTraCR')
  subdir('include/pytracr')
endif

####### Build test / example targets only if this repo is being loaded not as a subproject

if meson.is_subproject() == false

  # Build example targets
  if get_option('buildExamples')
    subdir('examples')
  endif

  # Build example targets
  if get_option('buildTests')
    subdir('tests')
  endif

endif
