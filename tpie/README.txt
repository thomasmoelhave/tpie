
THIS IS THE TPIE README FILE


Please visit http://www.cs.duke.edu/TPIE for the latest information on
TPIE. The TPIE manual, this README file, a list of changes since the
last release, and the main distribution tar file are available
separately from the TPIE web site.

The manual for TPIE will be created as file "tpie.ps" in the directory
tpie_0.9.01b/doc by the unzip/tar step below.  The manual includes
additional details regarding the installation procedure for TPIE.
----------------------------------------------------------------------

              INSTALLATION INSTRUCTIONS
              =========================

Place "tpie_0.9.01b.tgz" in the directory in which TPIE is to be
installed, "cd" into that directory, and execute the command

   tar xzf tpie_0.9.01b.tgz

or 

   gunzip -c tpie_0.9.01b.tgz | tar xvf -  

This will produce a directory "tpie_0.9.01b" with subdirectories
"doc", "include", "lib", "lib/src", and "test".  Enter the directory
"tpie_0.9.01b".  You must now configure TPIE for your particular
system.  To do this, use the command

   ./configure

The configuration program will produce the various Makefiles and
configuration files required to build TPIE on your system.  When this
is done, invoke your version of GNU "make":

   make all

to build the complete TPIE system.  This will build the components of
TPIE that must be tailored to your system. This includes: the TPIE
run-time library "tpie_0.9.01b/lib/libtpie.a", the test and sample
programs in directory "tpie_0.9.01b/test", and certain header files in
"tpie_0.9.01b/include".

You should now have a complete TPIE system, consisting of the
directories "include", "doc", "lib", "lib/src", "test".

The manual for TPIE will be created as file "tpie.ps" in the directory
tpie_0.9.01b/doc by the unzip/tar step above. It is also available
separately from the TPIE web site, http://www.cs.duke.edu/TPIE.

If you have problems, please consult the manual; there are some
requirements listed. Contact <tpie@cs.duke.edu> if you encounter other
problems.

#	$Id: README.txt,v 1.3 1999-11-19 23:12:55 hutchins Exp $	
