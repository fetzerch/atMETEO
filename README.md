atMETEO - An ATmega based weather station
=========================================

As its name implies *atMETEO* is a project for collecting and measuring weather
related data from different sensors. The data is being collected by an ATmega
based target and sent to a computer for further processing. From there
the data can for example be feed into [graphite](http://graphite.wikidot.com)
in order to create graphs or passed on to a home automation server.


Building
--------
The project uses CMake as configuration tool. Hence the very first in the build
chain is to execute CMake as shown below. Note that the default
configuration builds the project for execution on the build machine,
mainly for testing purposes.

    $ mkdir <build dir>
    $ cd <build dir>
    $ cmake <source dir>

### Cross compilation & flashing
A different configuration is needed to setup cross compilation and flashing for
ATmega based targets. For this purpose [cmake-avr] has to be available on
the build machine.

In order to use [cmake-avr] the `CMAKE_TOOLCHAIN_FILE` variable has to be set
as well as a few other configuration variables.

Exemplary usage for an Arduino Uno:

    $ cd <build dir>
    $ cmake -DCMAKE_TOOLCHAIN_FILE=<cmake-avr dir>/generic-gcc-avr.cmake \
            -DTARGET_CONFIGURATION=arduino -DAVR_MCU=atmega328p \
            -DAVR_PROGRAMMER=arduino -DAVR_UPLOADTOOL_PORT=/dev/ttyACM0 \
            <source dir>
    $ make && make upload_sensors

Please see the cmake-avr documentation for more configuration options.

[cmake-avr]: https://github.com/mkleemann/cmake-avr

### Unit tests
The project ships with a number of unit tests to ensure that the platform
independent code functions properly. The unit tests are meant to be
executed on the build system with `make test`.

Testing is enabled by default and can be disabled with the CMake option
`BUILD_TESTING`. The tests log to standard out and generate JUnit XML output
for further processing.

Code coverage information generation can be configured with the CMake
option `BUILD_CODECOVERAGE`. After the compilation with `make`, the generated
code coverage data can be converted into XML using `make gcovr_to_cobertura`.

### API Documentation
The Doxygen based API documentation can be build with `make dox`.
If not needed, the documentation support can be disabled with the CMake
option `BUILD_DOCUMENTATION`.


Disclaimer
----------
A micro controller project using C++ and templates is worth a disclaimer.
The idea for the project evolved while I was looking for a smaller
spare time project involving micro controllers and that can serve as
practical accompaniment while reading [Modern C++ Design] and [C++ Templates].

Thus the project doesn't claim to have the best, fastest or smallest
implementation. The goal was more to modularize parts to be reusable and easy
to extend and maintain.

[Modern C++ Design]: http://erdani.com/index.php/books/modern-c-design
[C++ Templates]: http://www.josuttis.com/tmplbook/tmplbook.html


License
-------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[GNU General Public License](http://www.gnu.org/licenses/gpl-2.0.html)
for more details.
