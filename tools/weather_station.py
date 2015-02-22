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

import argparse
import datetime
import json
import serial
import time
import threading


class SerialReceiverThread(threading.Thread):
    """ Thread that reads from serial port """

    def __init__(self, port, num_read=-1):
        """ Initialization """
        threading.Thread.__init__(self)
        self._serial = serial.Serial(port, 9600, timeout=3)
        self._num_read = num_read
        self._handler = []
        self._stop = threading.Event()
        self._last_received_time = time.time()

    def run(self):
        """ Execute thread """
        while not self.stopped():
            try:
                line = self._serial.readline().rstrip('\n')

                if len(line) > 0:
                    self._call_handler(line)
                    self._last_received_time = time.time()
                    if self._num_read > 0:
                        self._num_read -= 1

            except serial.serialutil.SerialException as err:
                print("Disconnected: %s" % err)
                self._serial.close()
                return

    def add_handler(self, handler):
        """ Add handler that is called whenever data has been read """
        self._handler.append(handler)

    def _call_handler(self, line):
        """ Call all registered handlers """
        for handler in self._handler:
            handler(self._elapsed_time(), line)

    def _elapsed_time(self):
        """ Elapsed time since data has been received """
        return time.time() - self._last_received_time

    def stop(self):
        """ Stop thread """
        self._stop.set()

    def stopped(self):
        """ Check if thread is stopped or is being stopped """
        return self._stop.isSet() or self._num_read == 0


class CommandLineClient(object):
    """ Command line client """

    @classmethod
    def _parse_arguments(cls):
        """ Parse command line arguments """
        parser = argparse.ArgumentParser()

        # General
        parser.add_argument('--serial-port', type=str, default='/dev/ttyACM0',
                            help="Serial port to connect to target")
        parser.add_argument('--reconnect-timeout', type=int, default=-1,
                            help="Reconnect serial connection")
        parser.add_argument('--num-read', type=int, default=-1,
                            help="Stop program after specified reads")
        parser.add_argument('--timeout', type=int, default=-1,
                            help="Stop program after specified timeout (in s)")

        # Graphite
        parser_gra = parser.add_argument_group('Graphite')
        parser_gra.add_argument('--graphite', action='store_true',
                                help="Send metrics to graphite server")
        parser_gra.add_argument('--graphite-server', type=str,
                                default='localhost', help="Graphite server")
        parser_gra.add_argument('--graphite-name', type=str, default='weather',
                                help="Graphite prefix and system_name")

        args = parser.parse_args()
        return args

    def __init__(self):
        """ Initialization """
        self._args = self._parse_arguments()
        self._serial_thread = None

    @classmethod
    def _console_serial_handler(cls, elapsed_time, line):
        """ Console output """
        print("%s: %3d: %s" % (datetime.datetime.now(), elapsed_time, line))

    def _graphite_serial_handler(self, _, line):
        """ Send metrics to graphite server """
        try:
            metrics = json.loads(line)
            print("Sending data to graphite: %s" % metrics)
            try:
                import graphitesend
                graphite = graphitesend.init(
                    graphite_server=self._args.graphite_server,
                    prefix=self._args.graphite_name,
                    system_name=self._args.graphite_name)
                graphite.send_dict(metrics)
                graphite.disconnect()
            except graphitesend.GraphiteSendException as err:
                print("Error: %s" % err)
        except ValueError as err:
            pass

    def start(self):
        """ Start receiver """
        if self._args.timeout > -1:
            timer = threading.Timer(self._args.timeout, self.stop)
            timer.start()

        while True:
            # Setup serial port
            try:
                self._serial_thread = SerialReceiverThread(
                    port=self._args.serial_port, num_read=self._args.num_read)
            except serial.serialutil.SerialException as err:
                print("Error: %s" % err)
                return 1

            self._serial_thread.add_handler(self._console_serial_handler)
            if self._args.graphite:
                self._serial_thread.add_handler(self._graphite_serial_handler)

            # Start receive
            try:
                self._serial_thread.start()
                while self._serial_thread.isAlive():
                    time.sleep(1)

                if self._args.reconnect_timeout >= 0:
                    print("Restarting in %d s" % self._args.reconnect_timeout)
                    time.sleep(self._args.reconnect_timeout)
                    continue

            except serial.serialutil.SerialException as err:
                print("Serial port error: %s" % err)
                if self._args.reconnect_timeout >= 0:
                    print("Restarting in %d s" % self._args.reconnect_timeout)
                    time.sleep(self._args.reconnect_timeout)
                    continue

            return

    def stop(self):
        """ Stop receiver """
        if self._serial_thread is not None:
            self._serial_thread.stop()


if __name__ == '__main__':
    CLI = CommandLineClient()
    try:
        CLI.start()
    except KeyboardInterrupt:
        print("Stopping")
        CLI.stop()
