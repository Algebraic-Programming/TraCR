project('ovni_extension', [ 'cpp', 'c'],
  subproject_dir : 'extern',
  default_options : [
  'cpp_std=c++11'
  ]
)

####### Storage for Instrumentation dependencies

InstrumentationDependencies = [ ]

# Warning handling option
warningAsErrorFlags=[]
if get_option('compileWarningsAsErrors') == true
  warningAsErrorFlags=[ '-Werror' ]
endif

InstrumentationBuildCppArgs = [
 '-Wfatal-errors',
  warningAsErrorFlags
 ]

InstrumentationBuildIncludes = include_directories([
   'include'
  ])
  

####### Build ovni part

ovniDependencies = [ ]
subdir('include/ovni')

InstrumentationDependencies += [ovniDependencies]

####### Collect the dependencies

TaskRBuildDep = declare_dependency(
    compile_args: TaskRBuildCppArgs,
    include_directories: TaskRBuildIncludes,
    dependencies: InstrumentationDependencies
    )

####### Build test / example targets only if this repo is being loaded not as a subproject

if meson.is_subproject() == false

  # Build example targets
  if get_option('buildExamples')
  subdir('examples')
  endif

endif
