project('TraCR', [ 'cpp', 'c'],
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
   'include'
  ])
  

####### Build ovni

cmake = import('cmake')

ovni_proj = cmake.subproject('ovni')
ovniDep = ovni_proj.dependency('ovni-static')  # or the actual target name

parson_dep = ovni_proj.dependency('parson-static')
common_dep = ovni_proj.dependency('common-static')

# add_project_arguments('-fPIC', language : 'cpp') # This is needed if ovni is build with the SHARED method
# add_project_arguments('-fPIC', language: 'c')

# message(ovni_proj.targets)

####### Collect the dependencies

InstrumentationBuildDep = declare_dependency(
    compile_args: InstrumentationBuildCppArgs,
    include_directories: InstrumentationBuildIncludes,
    dependencies: [ovniDep, parson_dep, common_dep]
    )

####### Build PyTraCR
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
