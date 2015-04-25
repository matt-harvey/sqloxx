Legal
=====

Copyright 2013 Matthew Harvey

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Overview
========

Sqloxx is a software library written in C++, using C++11 features. It
provides a high level wrapper around the SQLite database library, and
includes facilities for creating classes of persistent objects, and
managing these persistent objects by means of an in-memory cache.

The library was developed by Matthew Harvey, originally for use in his own
projects, and is released in the hope that it will be useful to others.

Dependencies
============

Sqloxx is written in standard C++, and utilizes some C++11 features. To build
and install the library, you will need:

- A reasonably conformant C++ compiler and standard library implementation
  (Sqloxx has been successfully built with GCC 4.7.2 and with later
  versions; it has not been tested with other compilers)

- CMake (version 2.8 or later)

- The Jewel C++ libraries (version 1.0.0 or later)

- The Boost C++ libraries (version 1.53.0 or later)

- UnitTest++ (known to work with version 1.4)

- A Tcl interpreter (known to work with version 8.6) (to run the test
  driver)

To build the API documentation, you will need:

- Doxygen (known to work with version 1.8.3.1)

At the time of writing, these dependencies can be obtained from the following
locations:
    
:CMake:      http://www.cmake.org
:Boost:         http://www.boost.org
:Jewel:      https://github.com/skybaboon/jewel
:UnitTest++: http://unittest-cpp.sourceforge.net
:Tcl:        http://tcl.tk
:Doxygen:     http://www.stack.nl/~dimitri/doxygen

Sqloxx also depends on SQLite; however the Sqloxx source distribution includes
the SQLite C source files directly, so it is not necessary to install SQLite
separately.

Initial build configuration
===========================

Open a command line and ``cd`` to the project root.

On Unix-like systems, enter::

    cmake -i

(On Windows, you will need to use the ``-G`` option to choose a Makefile
generator, or else run ``cmake-gui.exe .``. See CMake documentation for further
details.)

You will be prompted with ``Would you like to see the advanced options? [No]:``.
Answer ``n``, then follow the prompts.

(If you run into trouble with the build or have atypical requirements, you can
return to this step and answer ``y`` to configure more detailed build options.)

If in doubt about a particular option, it is generally best simply to hit enter
and keep the default setting for the option.

Note the options ``ENABLE_ASSERTION_LOGGING`` and ``ENABLE_EXCEPTION_LOGGING``.
These determine whether the ``JEWEL_ENABLE_ASSERTION_LOGGING`` and
``JEWEL_ENABLE_EXCEPTION_LOGGING`` macros will be defined within the compiled
Sqloxx library (regardless of whether they are defined in client code).
If in doubt, it is recommended to leave these logging options ``ON``.
(For more information on the significance of these macros, see the documentation
for the ``jewel::Log`` class, in the Jewel library.)

To build, test and install in one go
====================================

At the project root, enter::
    
    make install

If on a Unix-like system, you may need to run this as root, i.e.::

    sudo make install

This will cause the library and tests to be built (if not built already), and
will cause the tests to be run, with the results output to the console.

If and only if all the tests succeed, installation of the library and headers
will then proceed. For your information, a list of
the installed files will be saved in the file ``install_manifest.txt`` in the
project root.

If any tests fail, you are strongly encouraged to send the library developer
your test output, along with the file ``test.log`` (which should appear in the
project root), and the details of your system and build environment. (See
Contact_ for contact details.)

A list of the installed files will be saved in the
file ``install_manifest.txt`` in the project root. As there is no
"uninstall" target, this may be helpful in future for locating files to be
removed manually should you ever wish to uninstall the library.

To uninstall
============

There is no "make uninstall" target. However, it is straightforward to
uninstall the library manually. Locate the
file ``install_manifest.txt`` file that was created in the project directory
during installation.
This lists the files that were created during installation. Uninstalling the
library is a matter of removing these files.

To generate the documentation
=============================

If you have Doxygen installed and want to generate the API documentation, then
enter the following at the project root::

    make docs

HTML documentation will then be generated in the project root directory,
under ``html`` and can be browsed by opening the following file in your
web browser::

    html/index.html

Almost all of the documentation is generated from Doxygen markup in the
C++ headers themselves; so an alternative source of information on the Sqloxx
API, is simply to examine the headers directly.

Other build targets
===================

To clean build
--------------

Go to the project root and enter::

    make clean

This will clean all build targets from the project root, including
the source tarball (see below) if present, but *not* including the
HTML documentation. This is due to a quirk of CMake. To remove the
HTML documentation, simply manually delete the ``html`` directory from the
project root.

Note this will *not* cause the library to be uninstalled from the host system.

To build the library without installing or testing
--------------------------------------------------

At the project root, enter::

    make sqloxx


To build and run the test suite without installing
--------------------------------------------------

At the project root, enter::

    make test

After the test driver executable is built, the tests will automatically be run
and the results displayed.

If any tests fail, you are strongly encouraged to send the library developer
your test output, along with the file ``test.log`` (which should appear in the
project root), and the details of your system and build environment. (See
below for contact details.)

To build a source package for distribution
------------------------------------------

If you are running a Unix-like system, and have a ``tar`` program installed,
you can build a tarball of the library sources by entering the following
at the project root::
    
    make package

The tarball will appear in the project root directory, and will overwrite any
existing tarball with the same name.

As a safety measure, running ``make package`` always causes the tests to be
built and run, prior to the package being built. The package will not be built
unless all the tests pass.

To build multiple targets in one go
-----------------------------------

To build the library, build the tests and run the tests with one command, go to
the project root, and enter::

    make

Note this will *not* install the library, will *not* generate the documentation
and will *not* build a source tarball.

Contact
=======

sqloxx@matthewharvey.net

