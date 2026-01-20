project('TraCR', [ 'cpp' ],
  subproject_dir : 'extern',
  default_options : [
  'cpp_std=c++17',
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
   'include', 'extern'
  ])

####### Collect the dependencies

InstrumentationBuildDep = declare_dependency(
    compile_args: InstrumentationBuildCppArgs,
    include_directories: InstrumentationBuildIncludes
    )

####### Post-processing script

subdir('postprocessing')

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
