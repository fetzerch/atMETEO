atMETEO - An ATmega based weather station
=========================================

As its name implies *atMETEO* is a project for collecting and measuring weather
related data from different sensors. The data is being collected by an ATmega
based target and sent to a computer for further processing. From there
the data can for example be feed into [graphite](http://graphite.wikidot.com)
in order to create graphs or passed on to a home automation server.


Hardware
--------
The software can be built for different ATmega based target boards,
such as the [Arduino Uno]. The sensor access algorithms are implemented
hardware independently (as libsensors). This leads to a small hardware
dependent part, mainly for the pin connections, that can be
easily ported to different targets.

[Arduino Uno]: http://arduino.cc/en/pmwiki.php?n=Main/arduinoBoardUno


Compatible sensors
------------------

### Hideki Thermo/Hygrometer
The Hideki Thermo/Hygrometer (TS53) provides temperature and humidity over an
wireless RF 433 MHz interface. In order to collect this sensor data, an
RF 433 MHz receiver has to be connected to the targets Input Capture Pin (ICP).

### DHT22 / AM2302 Temperature and humidity module
The DHT22 / [AM2302] is a digital temperature and humidity sensor. The sensor
has to be connected to one of the digital I/O pins (default AD2).

[AM2302]: http://akizukidenshi.com/download/ds/aosong/AM2302.pdf

### Bosch BMP180 Digital pressure sensor
The [Bosch BMP180] is a digital pressure sensor providing ambient temperature
and barometric pressure. The sensor has to be conencted to the I2C bus.

[Bosch BMP180]: http://ae-bst.resource.bosch.com/media/products/dokumente/bmp180/BST-BMP180-DS000-12~1.pdf

### Melexis MLX90614 Infrared thermometer
The [Melexis MLX90614] is an infrared thermometer providing ambient and object
temperatures. The sensor has to be connected to the I2C bus.

[Melexis MLX90614]: http://www.melexis.com/Asset/IR-sensor-thermometer-MLX90614-Datasheet-DownloadLink-5152.aspx

### Figaro TGS 2600
The [Figaro TGS 2600] is a high sensitive high sensitivity air contaminant
sensor. Due to its analog nature, the detected uncalibrated sensor resistance
has to be processed according to the datasheet. For collecting data from
this sensor, it has to be connected to the Analog to Digital Conversion Pin 0
(ADC0) using a load resistance of 10k Ohm.

[Figaro TGS 2600]: http://www.figarosensor.com/products/2600pdf.pdf


Output format
-------------
The received sensor values are transmitted over the UART interface and / or
over Ethernet using UDP messages as JSON object.

Exemplary data:

    {"rf433_1": {"temperature":-5.0,"humidity":48,"battery":true}}
    {"rf433_2": {"temperature"18.3,"humidity":45,"battery":false}}
    {"dht22": {"temperature":10.0,"humidity":32.0}}
    {"bmp180": {"temperature": 17.3,"pressure": 1008.2}}
    {"mlx90614": {"ambient_temperature":22.3,"object_temperature":34.6}}
    {"tgs2600": {"sensor_resistance":14000}}


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
