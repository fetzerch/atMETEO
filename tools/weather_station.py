#!/usr/bin/env python

# atMETEO - An ATmega based weather station
# Copyright (C) 2014-2015 Christian Fetzer
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

""" Command line client """

from __future__ import print_function

import abc
import argparse
import datetime
import json
import math
import re
import socket
import time
import threading

import pywws.conversions
import serial


class ReceiverThread(threading.Thread):  # pragma: no cover
    """ Generic Thread for receiving data asynchronously """
    __metaclass__ = abc.ABCMeta

    def __init__(self, port, reconnect_timeout):
        """ Initialization """
        threading.Thread.__init__(self)
        self._connection = None
        self._port = port
        self._reconnect_timeout = reconnect_timeout
        self._handler = []
        self._stop = threading.Event()

    @abc.abstractmethod
    def initialize(self):
        """ Initialize connection """

    @abc.abstractmethod
    def receive(self):
        """ Receiver loop """

    def handle_received(self, source, data):
        """ Handle received data """
        line = data.rstrip('\n')
        if line:
            for handler in self._handler:
                handler(source, line)

    def run(self):
        """ Execute thread """
        while not self.stopped():
            try:
                if self._connection is None:
                    self.initialize()
                self.receive()
            except IOError as err:
                print("Error: %s" % err)
                if self._reconnect_timeout >= 0:
                    print("Restarting in %d s" % self._reconnect_timeout)
                    time.sleep(self._reconnect_timeout)
                else:
                    self.stop()

    def add_handler(self, handler):
        """ Add handler that is called whenever data has been read """
        self._handler.append(handler)

    def stop(self):
        """ Stop thread """
        self._stop.set()

    def stopped(self):
        """ Check if thread is stopped or is being stopped """
        return self._stop.isSet()


class UdpReceiverThread(ReceiverThread):  # pragma: no cover
    """ Thread that reads from UDP port """

    def __init__(self, port, reconnect_timeout):
        ReceiverThread.__init__(self, port, reconnect_timeout)

    def initialize(self):
        try:
            self._connection = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._connection.bind(('0.0.0.0', self._port))
            self._connection.settimeout(3)
        except socket.error as err:
            self._connection = None
            raise IOError(err)

    def receive(self):
        try:
            data, source = self._connection.recvfrom(1024)
            self.handle_received(source[0], data)
        except socket.timeout:
            pass


class SerialReceiverThread(ReceiverThread):  # pragma: no cover
    """ Thread that reads from serial port """

    def __init__(self, port, reconnect_timeout):
        ReceiverThread.__init__(self, port, reconnect_timeout)

    def initialize(self):
        try:
            self._connection = serial.Serial(self._port, 9600, timeout=3)
        except serial.serialutil.SerialException as err:
            raise IOError(err)

    def receive(self):
        try:
            self.handle_received(self._port, self._connection.readline())
        except serial.serialutil.SerialException as err:
            self._connection.close()
            self._connection = None
            raise IOError(err)


class RoomMapping(object):
    # pylint: disable=bad-option-value,useless-object-inheritance
    """ Maps a sensor to a room based on a mapping string

        e.g. 'study:*, garden:rf433_1' """

    def __init__(self, mapping_string):
        """ Instanciate room mapping class """
        self._mapping = self._prepare_mapping_table(mapping_string)

    @classmethod
    def _prepare_mapping_table(cls, mapping_string):
        """ Returns mapping table in form: [(compiled regex, prefix), ...] """
        mapping = [x.strip().partition(':')
                   for x in mapping_string.split(',')]
        mapping = [(re.compile('^{}$'.format(regex.replace('*', '.*')
                                             if regex != '' else '.*')),
                    prefix) for prefix, _, regex in mapping[::-1]]
        return mapping

    def get_mapping_table(self):
        """ Return the mapping table for testing """
        return self._mapping

    def get_room_mapping(self, sensor):
        """ Return the room associated with the sensor """
        return next((prefix for regex, prefix in self._mapping
                     if regex.search(sensor)))  # pragma: no cover


def test_room_mapping():
    """ Tests mapping prefixes """
    mapping = RoomMapping('')
    assert mapping.get_mapping_table() == [(re.compile('^.*$'), '')]
    assert mapping.get_room_mapping('anysensor') == ''

    mapping = RoomMapping('study')
    assert mapping.get_mapping_table() == [(re.compile('^.*$'), 'study')]
    assert mapping.get_room_mapping('anysensor') == 'study'

    mapping = RoomMapping('study:*')
    assert mapping.get_mapping_table() == [(re.compile('^.*$'), 'study')]
    assert mapping.get_room_mapping('anysensor') == 'study'

    mapping = RoomMapping('study:*, garden:rf433*')
    assert mapping.get_mapping_table() == \
        [(re.compile('^rf433.*$'), 'garden'),
         (re.compile('^.*$'), 'study')]
    assert mapping.get_room_mapping('anysensor') == 'study'
    assert mapping.get_room_mapping('rf433') == 'garden'

    mapping = RoomMapping('garden:rf433*, study:*')
    assert mapping.get_mapping_table() == \
        [(re.compile('^.*$'), 'study'),
         (re.compile('^rf433.*$'), 'garden')]
    assert mapping.get_room_mapping('anysensor') == 'study'
    assert mapping.get_room_mapping('rf433') == 'study'

    mapping = RoomMapping('study:*, garden:rf433*:incorrect')
    assert mapping.get_mapping_table() == \
        [(re.compile('^rf433.*:incorrect$'), 'garden'),
         (re.compile('^.*$'), 'study')]


class CommandLineClient(object):  # pragma: no cover
    # pylint: disable=bad-option-value,useless-object-inheritance
    """ Command line client """

    @classmethod
    def _parse_arguments(cls):
        """ Parse command line arguments """
        parser = argparse.ArgumentParser()

        # Serial connection
        parser.add_argument('--serial', action='store_true',
                            help="Receive metrics from serial port")
        parser.add_argument('--serial-port', type=str, default='/dev/ttyACM0',
                            help="Serial port to connect to target")
        parser.add_argument('--reconnect-timeout', type=int, default=-1,
                            help="Reconnect serial connection")

        # Network connection (UDP)
        parser.add_argument('--udp', action='store_true',
                            help="Receive metrics from UDP port")
        parser.add_argument('--udp-port', type=int, default=8600,
                            help="UDP port to connect to bind to")

        # Graphite
        parser_gra = parser.add_argument_group('Graphite')
        parser_gra.add_argument('--graphite', action='store_true',
                                help="Send metrics to graphite server")
        parser_gra.add_argument('--graphite-server', type=str,
                                default='localhost', help="Graphite server")
        parser_gra.add_argument('--graphite-name', type=str, default='weather',
                                help="Graphite prefix and system_name")

        # Generic
        parser_gra.add_argument('--room-mapping', type=str, default=None,
                                help="Room mapping"
                                     "(e.g. 'study:*, garden:rf433_1')")

        args = parser.parse_args()
        return args

    def __init__(self):
        """ Initialization """
        self._args = self._parse_arguments()
        self._mapping = RoomMapping(self._args.room_mapping)
        self._receive_threads = []
        self._outdoor_values = None

    def _preprocess_metrics(self, line):
        """ Preprocess metrics """

        def index(sequence):
            """ Join not None items in sequence with '.' separator. """
            return '.'.join([item for item in sequence if item is not None])

        result = {}
        try:
            metrics = json.loads(line)
            for sensor, data in metrics.items():
                sensor_prefix = self._mapping.get_room_mapping(sensor)
                sensor = re.sub(r'_\d+$', '', sensor)
                for metric, value in data.items():
                    result[index((sensor_prefix, sensor, metric))] = value

                # Add count metric for graphite
                result[index((sensor_prefix, sensor, 'count'))] = 1

                # Calculate dewpoint
                try:
                    result[index((sensor_prefix, sensor, 'dewpoint'))] = \
                        round(pywws.conversions.dew_point(
                            data['temperature'], data['humidity']), 2)
                except (KeyError, TypeError):
                    pass

                # Calculate absolute humidity
                # See: https://www.kompf.de/weather/vent.html
                # See: https://www.wetterochs.de/wetter/feuchte.html
                try:
                    def _humidity_abs(temperature, humidity):
                        # Compute saturated water vapor pressure in hPa
                        svp = 6.112 * math.exp((17.67 * temperature) /
                                               (243.5 + temperature))
                        # Compute actual water vapor pressure in hPa
                        avp = humidity / 100.0 * svp
                        # Compute the absolute humidity in g/m3
                        aah = 10**5 * 18.016/8314.3 * avp / \
                            (temperature + 273.15)
                        return round(aah, 2)
                    result[index((sensor_prefix, sensor, 'humidity_abs'))] = \
                        _humidity_abs(data['temperature'], data['humidity'])
                except (KeyError, TypeError):
                    pass

                # Calculate wall temperature and humidity for indoor sensors
                # using saved values of the outdoor sensor.
                # See: https://forum.fhem.de/index.php?topic=29773.0
                if sensor_prefix == 'garden':
                    self._outdoor_values = data
                elif self._outdoor_values is not None:
                    try:
                        k = 0.73
                        wall_temperature = k * data['temperature'] + \
                            (1-k) * self._outdoor_values['temperature']

                        wall_humidity = \
                            (data['humidity'] * 10**(
                                (7.62 * data['temperature']) /
                                (234.175 + data['temperature'])) *
                             (273.15 + wall_temperature)) / \
                            (10**((7.62 * wall_temperature) /
                                  (234.175 + wall_temperature)) *
                             (273.15 + data['temperature']))

                        result[index((sensor_prefix, sensor,
                                      'wall_temperature'))] = \
                            round(wall_temperature, 2)

                        result[index((sensor_prefix, sensor,
                                      'wall_humidity'))] = \
                            round(wall_humidity, 2)
                    except (KeyError, TypeError):
                        pass

        except (ValueError, AttributeError):
            print("Error parsing line: %s" % line)
        return result

    @classmethod
    def _console_handler(cls, source, line):
        """ Console output """
        print("%s: %s: %s" % (datetime.datetime.now(), source, line))

    def _graphite_handler(self, _, line):
        """ Send metrics to graphite server """
        metrics = self._preprocess_metrics(line)
        print("Sending data to graphite: %s" % metrics)
        try:
            import graphitesend \
                # pylint: disable=bad-option-value,import-outside-toplevel
            graphite = graphitesend.init(
                graphite_server=self._args.graphite_server,
                prefix=self._args.graphite_name,
                system_name=self._args.graphite_name)
            graphite.send_dict(metrics)
            graphite.disconnect()
        except (ImportError, graphitesend.GraphiteSendException) as err:
            print("Error: %s" % err)

    def start(self):
        """ Start receiver """

        # Setup serial thread
        if self._args.serial:
            self._receive_threads.append(SerialReceiverThread(
                self._args.serial_port, self._args.reconnect_timeout))

        # Setup UDP thread
        if self._args.udp:
            self._receive_threads.append(UdpReceiverThread(
                self._args.udp_port, self._args.reconnect_timeout))

        # Assign handler and start receiving
        for thread in self._receive_threads:
            thread.add_handler(self._console_handler)
            if self._args.graphite:
                thread.add_handler(self._graphite_handler)
            thread.start()

        while True:
            keepalive = False
            for thread in self._receive_threads:
                if thread.isAlive():
                    keepalive = True
                    break
            if keepalive:
                time.sleep(1)
            else:
                return

    def stop(self):
        """ Stop receiver """
        for thread in self._receive_threads:
            thread.stop()


if __name__ == '__main__':  # pragma: no cover
    CLI = CommandLineClient()
    try:
        CLI.start()
    except KeyboardInterrupt:
        print("Stopping")
        CLI.stop()
