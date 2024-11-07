# ovni Backend Configuration
# used to link the ovni library

# Get ovni
ovniDep = dependency('ovni', required: false)
    
# If not installed globally, find pkg-config
# If also not found, then trigger a failure
if ovniDep.found() == false
  ovniDep = dependency('ovni', method: 'pkg-config', required: true)
endif

ovniDependencies += declare_dependency(
    dependencies: ovniDep,
)    

##### linking the ovni_extention as well

library_name = 'foo'  # Name of your library

# List of source files for the library
library_sources = [
  'instrumentation.hpp',
]

# Define the library target
my_lib = library(library_name, 
    library_sources,
    install : true,     # Install the library
    install_dir : get_option('libdir')  # Install location
)

# Specify the include directory
install_headers('foo.hpp', subdir : 'include')  # Installs as <include_path>/foo.hpp

# Optional: Create a `foo_dep` dependency object for users of your library
foo_dep = declare_dependency(
  include_directories : include_directories('.'),
  link_with : my_lib
)

# This line is optional but useful if you're building this library as a subproject
# so that it can be used in other projects via dependency('foo')
meson.override_dependency(library_name, foo_dep)