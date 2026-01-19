
# Copy state.cfg to the build directory
configure_file(
  input : 'paraver/state.cfg',
  output : 'state.cfg',
  copy : true
)

executable('tracr_process', 'tracr_process.cpp', dependencies: InstrumentationBuildDep)