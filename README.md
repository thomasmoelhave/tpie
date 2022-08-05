# TPIE - The Templated Portable I/O Environment

[![LGPL-3.0 License](https://img.shields.io/badge/license-LGPL%203.0-blue.svg)](COPYING.md)
[![unit tests](https://github.com/thomasmoelhave/tpie/actions/workflows/unit_test.yml/badge.svg)](https://github.com/thomasmoelhave/tpie/actions/workflows/unit_test.yml)
[![documentation](https://github.com/thomasmoelhave/tpie/actions/workflows/doxygen.yml/badge.svg)](https://thomasmoelhave.github.io/tpie)

The TPIE (Templated Portable I/O Environment) library is a tool box providing
efficient and convenient tools to ease the implementation of algorithms and data
structures on very large sets of data.

**Project status**

As of 2021, this project does not see much development, but it is used heavily
in production. There has not been a TPIE release for a long time, but the master
branch is very stable and is unlikely to see major breaking changes.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Documentation](#documentation)
- [Usage](#usage)
    - [In your project](#in-your-project)
    - [Unit tests](#unit-tests)
    - [Examples](#examples)
- [License](#license)
<!-- markdown-toc end -->

## Documentation

For help, installation, usage and documentation, visit the
[TPIE homepage](http://www.madalgo.au.dk/tpie/); the API documentation is also
hosted [there](http://www.madalgo.au.dk/tpie/doc/) and on
[GitHub pages](http://www.thomasmoelhave.github.io/tpie).

## Dependencies

TPIE has a mandatory dependency on the _Boost_ library and requires a C++
compiler that supports the _14_ standard (e.g. GNU 7+ and Clang 10+).
Furthermore, optionally TPIE can use the _Snappy_, _LZ4_, and _ZSTD_
compression algorithms, if available.

## Usage
Building TPIE is done entirely with _CMake_.

### In your project

Assuming this repository is cloned into the _external/tpie_ folder of your
project, then add TPIE as a subdirectory in your own _CMakeLists.txt_.

```cmake
add_subdirectory (external/tpie tpie)
```

TPIE is then linked to each target executable of choice.

```cmake
add_executable(<target> <source>)
target_link_libraries(<target> tpie)
set_target_properties(<target> PROPERTIES CXX_STANDARD 14)
```

Then build your project with CMake. For other ways to install TPIE, see
[Compiling and installing TPIE](https://thomasmoelhave.github.io/tpie/setup.html)
in the documentation.

### Unit tests

To compile the entire project and run the unit tests execute the following
commands.

```bash
# Compile entire TPIE project
mkdir build
cd build
cmake -D CXX_STANDARD 14 ..

# Compile tests
make

# Run unit tests
ctest
```

### Examples

The _example/helloworld.cpp_ provides a rudementary example for how to use TPIE,
assuming it has been
[installed](https://thomasmoelhave.github.io/tpie/setup.html). The _apps/_
folder includes multiple non-trivial examples. To compile and run any of the
CMake enabled examples in _apps_ execute the following commands

```bash
# CMake
mkdir build
cd build
cmake -D CXX_STANDARD 14 ..

# Compile
cd apps/<app>/
make

# Run
./<app>
```


## License
The software and documentation files in this repository are provided under the
[LGPL 3.0 License](/COPYING.LESSER.md).

