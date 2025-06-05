# TraCR

**TraCR** (pronounced *tracer*) is a lightweight and user-friendly instrumentation library for tracing application behavior. It is designed to be easily toggled on or off at compile time, making it suitable for both development and production environments.

## Features

- **Lightweight integration** with minimal overhead when disabled.
- **Compile-time toggling** using `-DENABLE_INSTRUMENTATION`.
- **Optional debugging output** with `-DENABLE_DEBUG`.
- **Examples** provided in the `examples/` folder.
- **Post-processing scripts** for trace visualization in `python_scripts/`.

## Instrumentation Control

- `-DENABLE_INSTRUMENTATION` to enable instrumentation.
  - If undefined, all instrumentation calls become `void` (i.e. no performance impact).
- `-DENABLE_DEBUG` to enable debugging output from TraCR.

## Example Output

A sample trace generated using TraCR is shown below:

![TaskR running Fibonacci(15) on 8 workers](images/paraver_view1.png)

## Installation

### Step 1: Install Ovni (Dependency)

Clone this repo recursively as TraCR relies on [ovni](https://github.com/bsc-pm/ovni) as its core instrumentation backend. To install ovni:

```bash
git clone --recurse-submodules https://github.com/Algebraic-Programming/TraCR.git
cd tracr

export prefix=$HOME/library/ovni  # or your preferred installation path

mkdir -p extern/ovni/build
pushd extern/ovni/build
cmake .. -DCMAKE_INSTALL_PREFIX=$prefix
make -j$(nproc)
make install
popd
```

Add the following to your .bashrc to make ovni accessible:

```bash
# Ovni environment setup
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$HOME/library/ovni/lib/pkgconfig
export PATH=$HOME/library/ovni/bin:$PATH
```

To verify the installation, navigate to the build directory:

```bash
cd tracr/extern/ovni/build
make test
```

### Step 2: Install TraCR

At the moment, there is no standalone installation process for TraCR.  
However, the recommended way is to use it as a [Meson subproject](https://mesonbuild.com/Subprojects.html).  
To integrate it, add the following to your `meson.build` file:

```meson
# Add TraCR as a subproject
InstrumentationProject = subproject('tracr', required: true)
InstrumentationBuildDep = InstrumentationProject.get_variable('InstrumentationBuildDep')
<your_dependencies> += InstrumentationBuildDep

# Enable instrumentation
add_project_arguments('-DENABLE_INSTRUMENTATION', language: 'cpp')

# Uncomment the following line to enable debug prints from TraCR
# add_project_arguments('-DENABLE_DEBUG', language: 'cpp')
```

or in CMake something like this:

```cmake
# Add the TraCR subproject
add_subdirectory(external/tracr)

# Enable instrumentation
add_definitions(-DENABLE_INSTRUMENTATION)

# Optional: Enable debug prints
# add_definitions(-DENABLE_DEBUG)

# Link your target against the TraCR library
target_link_libraries(your_target PRIVATE tracr_instrumentation)
```

But then you would need to add a `CMakeLists.txt` in TraCR.

## Trace Visualization with Paraver

The generated traces are stored in a folder called `ovni/`. This one have to be emulated via `ovniemu ovni/`.

To visualize traces produced by TraCR, install [Paraver](https://tools.bsc.es/paraver):

```bash
wget https://ftp.tools.bsc.es/wxparaver/wxparaver-4.11.4-Linux_x86_64.tar.bz2
```

Unpack and follow the installation instructions from Paraver’s documentation.

## License

Copyright 2025 Huawei Technologies Co., Ltd.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
