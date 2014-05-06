TPIE is a library for manipulating large datasets on a desktop computer.
TPIE provides implementations of several external memory algorithms and data
structures as well as a framework for efficient and portable data processing.
Currently, TPIE provides implementations of the following algorithms and data
structures:

* External memory merge sorting
* Internal parallel quick sort
* Implementation of Sanders' fast priority queue for cached memory
* Simple buffered stacks and queues

These implementations are backed by the memory manager of TPIE that allows
the user to specify how much internal memory to use at most. The progress
reporting framework enables reporting accurate progress metrics.

TPIE is written in C++ and depends on the Boost C++ libraries. TPIE may be
compiled with GCC, Clang and MSVC, and due to its portable nature, it should be
easy to port TPIE to other systems if this is required.

Downloads
---------

To use TPIE, you need the library (`libtpie.a` on Linux,
`tpie.lib` on Windows) and the development headers.
These are provided as `.deb`-packages that may be installed on a
Debian-based system with `dpkg -i` or a similar tool;
as tarballs that may be installed into `/usr/local` or another installation prefix;
and as Git source checkouts, such that you may compile the library for yourself.

*June 11, 2013.*
TPIE 1.1 (Git tag `v1.1`):
[deb](downloads/tpie-1.1.0-Linux.deb)
[tarball](downloads/tpie-1.1.0-Linux.tar.gz)
[source](https://github.com/thomasmoelhave/tpie/tree/v1.1)

*June 5, 2013.*
TPIE 1.1 Release Candidate 1 (Git tag `v1.1rc1`):
[deb](downloads/tpie-1.1.0rc1-Linux.deb)
[tarball](downloads/tpie-1.1.0rc1-Linux.tar.gz)
[source](https://github.com/thomasmoelhave/tpie/tree/v1.1rc1)

*December 14, 2012.*
TPIE 1.0 (Git tag `v1.0`):
[deb](downloads/tpie-1.0.0-Linux.deb)
[tarball](downloads/tpie-1.0.0-Linux.tar.gz)
[source](https://github.com/thomasmoelhave/tpie/tree/v1.0)

*October 11, 2012.*
TPIE 1.0 Release Candidate 2 (Git tag `v1.0rc2`):
[deb](downloads/tpie-1.0.0rc2-Linux.deb)
[tarball](downloads/tpie-1.0.0rc2-Linux.tar.gz)
[source](https://github.com/thomasmoelhave/tpie/tree/v1.0rc2)

*August 2, 2012.*
TPIE 1.0 Release Candidate 1 (Git tag `v1.0rc1`):
[deb](downloads/tpie-1.0.0rc1-Linux.deb)
[tarball](downloads/tpie-1.0.0rc1-Linux.tar.gz)
[source](https://github.com/thomasmoelhave/tpie/tree/v1.0rc1)

If you want to hack on TPIE,
[check it out](https://github.com/thomasmoelhave/tpie) using the Git
command:

```git clone git://github.com/thomasmoelhave/tpie.git```

Mailing lists
-------------

TPIE development is driven in part by two public mailing lists.

* [tpie-devel](https://maillist.au.dk/mailman/listinfo/tpie-devel.madalgo)
  for development discussions and announcements
* [tpie-commits](https://maillist.au.dk/mailman/listinfo/tpie-commits.madalgo)
  for notifications when commits are pushed to GitHub
