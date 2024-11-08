# ovni Backend Configuration
# used to link the ovni library

# Get ovni
ovniDep = dependency('ovni', required: false)
    
# If not installed globally, find pkg-config
# If also not found, then trigger a failure
if ovniDep.found() == false
  ovniDep = dependency('ovni', method: 'pkg-config', required: true)
endif