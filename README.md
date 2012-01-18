TPIE - The Templated Portable I/O Environment
=============================================

The TPIE (Templated Portable I/O Environment) library is a tool box providing
efficient and convenient tools to ease the implementation of algorithm and data
structures on very large sets of data.

Installation on Linux
---------------------

You will need Git, Cmake and a C++ compiler in order to use TPIE. On a modern
Linux system, the following steps will suffice.

First, clone the [git repository](https://github.com/thomasmoelhave/tpie) and
checkout the branch or tag you wish to compile:

    git clone git://github.com/thomasmoelhave/tpie.git
    cd tpie
    git checkout master

Now, create a build directory and run Cmake followed by make.

    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE:STRING=Release ..
    make tpie

This will generate the static library `libtpie.a` in the `build/tpie` folder. To
link your application with TPIE, you will now want to add the TPIE root and the
build folder to your project's header file include path, and add the build/tpie
folder to your project's library path.

To run the unit tests, use CTest.

    make all
    ctest

The unit test executables are placed in the `test/unit` folder and are prefixed
with the token `ut-`.

Installation on Windows
-----------------------

As above, clone the [git repository](https://github.com/thomasmoelhave/tpie) at
`git://github.com/thomasmoelhave/tpie.git`, perhaps using TortoiseGit, and set
up the build environment using Cmake, perhaps using the Cmake GUI. TPIE is
known to work with the Microsoft Visual Studio 2008 and 2010 compilers. Be sure
to select a 64-bit build when your platform supports it.

Help
----

If you have problems compiling or using TPIE, consult the API documentation, or
file a [bug report on GitHub](https://github.com/thomasmoelhave/tpie/issues).

History
-------

Originally, TPIE was known as the Transparent Portable I/O Environment and was
hosted by [Duke University](http://www.cs.duke.edu/TPIE/). Later, the work has
been picked up by researchers at [MADALGO, Aarhus
University](http://www.madalgo.au.dk/Trac-tpie). As of 2012, the current
maintainer is [Mathias Rav](http://cs.au.dk/~rav/).
