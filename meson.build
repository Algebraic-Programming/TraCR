project('ovni_extension', [ 'cpp', 'c'],
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
   'src'
  ])
  

####### Build ovni and ovni_extension part

subdir('src')

if get_option('enableInstrumentations')
  add_project_arguments('-DUSE_INSTRUMENTATION', language: 'cpp')
endif

####### Collect the dependencies

InstrumentationBuildDep = declare_dependency(
    compile_args: InstrumentationBuildCppArgs,
    include_directories: InstrumentationBuildIncludes,
    dependencies: ovniDep
    )

####### Build test / example targets only if this repo is being loaded not as a subproject

if meson.is_subproject() == false

  # Build example targets
  if get_option('buildExamples')
  subdir('examples')
  endif

endif
