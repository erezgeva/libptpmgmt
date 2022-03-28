"""
This program continuously checks whether a PTP client is well synced to its master.

SPDX-License-Identifier: BSD-3-Clause
SPDX-FileCopyrightText: Czech Technical University in Prague
"""

import argparse
import contextlib
import ctypes
import sys

import numpy as np
import os
import pmc
import time

from collections import deque
from threading import Event, Lock, Thread

try:
    # noinspection PyCompatibility
    from exceptions import KeyboardInterrupt
except ImportError:
    pass


def get_bitmask_data(message):
    """
    Get the raw C-array representing SUBSCRIBE_EVENTS_NP.bitmask.

    :param pmc.SUBSCRIBE_EVENTS_NP_t message: The message to get bitmask of.
    :return: Reference to the bitmask.
    :note: This is a workaround for https://github.com/erezgeva/libpmc/issues/3 . 
    """
    if sys.version_info[0] == 2:
        raw_pointer = message.bitmask.__long__()
    else:
        raw_pointer = message.bitmask.__int__()
    return ctypes.cast(raw_pointer, ctypes.POINTER(ctypes.c_uint8))


class PeriodicTaskThread(Thread):
    """
    A thread that periodically calls some function and can be stopped between the calls.
    """
    def __init__(self, period, stop_event):
        """
        :param int period: Period of calling execute() (in seconds).
        :param Event stop_event: The event that can stop execution of this thread.
        """
        super(PeriodicTaskThread, self).__init__()
        self.period = period
        self._stop_event = stop_event

    def execute(self):
        """
        The task to be periodically called.
        """
        raise NotImplementedError()

    def run(self):
        while not self._stop_event.wait(self.period):
            if self._stop_event.is_set():
                break
            if not self.execute():
                self._stop_event.set()
                break


class SubscriptionRenewalThread(PeriodicTaskThread):
    """
    A thread that periodically renews subscription to SUBSCRIBE_EVENTS_NP.
    """
    def __init__(self, watchdog, stop_event):
        """
        :param PtpSyncWatchdog watchdog: The watchdog object.
        :param Event stop_event: The event that can stop execution of this thread.
        """
        assert isinstance(watchdog, PtpSyncWatchdog)
        super(SubscriptionRenewalThread, self).__init__(watchdog.subscribe_renewal, stop_event)
        self.watchdog = watchdog

    def execute(self):
        try:
            self.watchdog.subscribe_events()
        except BaseException as e:
            if not isinstance(e, KeyboardInterrupt):
                self.watchdog.print_error("Subscribe error (" + str(type(e)) + "): " + str(e))
            if not isinstance(e, Exception):
                return False
        return True


class CheckPathDelayThread(PeriodicTaskThread):
    """
    A thread that periodically calls GET CURRENT_DATA_SET so that path delay can be re-estimated.
    """
    def __init__(self, watchdog, stop_event):
        """
        :param PtpSyncWatchdog watchdog: The watchdog object.
        :param Event stop_event: The event that can stop execution of this thread.
        """
        assert isinstance(watchdog, PtpSyncWatchdog)
        super(CheckPathDelayThread, self).__init__(10, stop_event)
        self.watchdog = watchdog

    def execute(self):
        try:
            self.watchdog.send_get_message(pmc.CURRENT_DATA_SET)
        except BaseException as e:
            if not isinstance(e, KeyboardInterrupt):
                self.watchdog.print_error("Error requesting path delay (" + str(type(e)) + "): " + str(e))
            if not isinstance(e, Exception):
                return False
        return True


# noinspection PyClassHasNoInit
class PtpSyncQuality:
    """
    Quality of PTP sync.
    """

    UNKNOWN = 0
    NOT_SYNCED = 1
    UNSTABLE = 2
    STABLE = 3


class PtpSyncData:
    """
    Detailed data describing PTP sync status.
    """
    def __init__(self, num_offset_values, offset_threshold, offset_std_threshold=None):
        """
        :param int num_offset_values: Length of history.
        :param int offset_threshold: Values under this offset will be considered as good sync (in ns).
        :param int|None offset_std_threshold: If set, also standard deviation will be checked to be below this (in ns).
        """
        self.peer_mean_path_delay = None
        self.port_state = None
        self.gm_identity = None
        self.clock_id = None
        self.last_offset_time = None
        self._offsets = deque(maxlen=num_offset_values)
        self._offset_threshold = offset_threshold
        self._offset_std_threshold = offset_std_threshold
        self._offsets_lock = Lock()
        self._last_quality = PtpSyncQuality.UNKNOWN

    @property
    def offset(self):
        """
        Get the last offset from master (or None is none has been measured yet).
        :return: The offset.
        :rtype: int|None
        """
        with self._offsets_lock:
            if len(self._offsets) == 0:
                return None
            return self._offsets[-1]

    def get_offsets(self):
        """
        Get a copy of the history of offsets that are considered for sync quality estimation.
        :return: The list of offsets (newest last).
        :rtype: Tuple[int]
        """
        with self._offsets_lock:
            return list(self._offsets)

    def add_offset(self, value):
        """
        Add a new offset to the history.
        :param int value: Offset (in ns).
        """
        with self._offsets_lock:
            self._offsets.append(value)

    def get_sync_quality(self):
        """
        Estimate the sync quality based on the parameters given in constructor and the measured history of offsets.
        :return: The estimated quality.
        :rtype: int (PtpSyncQuality)
        """
        with self._offsets_lock:
            if len(self._offsets) == 0:
                return self._last_quality

            offsets = list(self._offsets)

        all_ok = all([abs(v) <= self._offset_threshold for v in offsets])
        std_ok = np.std(offsets) <= self._offset_std_threshold if self._offset_std_threshold is not None else True

        if not all_ok or not std_ok or len(offsets) < self._offsets.maxlen:
            if self._last_quality == PtpSyncQuality.STABLE:
                quality = PtpSyncQuality.UNSTABLE
            else:
                quality = PtpSyncQuality.NOT_SYNCED
        else:
            quality = PtpSyncQuality.STABLE

        if self.last_offset_time is not None and (time.time() - self.last_offset_time > 2.0):
            quality = PtpSyncQuality.UNKNOWN

        self.last_offset_time = time.time()
        self._last_quality = quality
        return quality

    def get_offset_str(self):
        """
        Get a string representing the estimated offset.
        :return: The offset string.
        :rtype: str
        """
        with self._offsets_lock:
            if len(self._offsets) == 0:
                return "N/A"
            mean = np.mean(self._offsets)
            std = np.std(self._offsets)
            worst = 0
            for v in self._offsets:
                if abs(v) > abs(worst):
                    worst = v
            return "%i +- %i ns (worst %i ns)" % (int(mean), int(std), worst)

    def get_offsets_str(self):
        """
        Get a string representing the whole history of remembered offsets.
        :return: The offsets.
        :rtype: str
        """
        with self._offsets_lock:
            return ", ".join(map(str, self._offsets))


class PtpSyncWatchdog:
    """
    A watchdog for PTP sync estimation.
    """
    def __init__(self, socket, ptp_config, offset_threshold=100, offset_std_threshold=None, num_offset_values=5,
                 callback=None, subscribe_duration=180, subscribe_renewal=60):
        """
        :param pmc.SockUnix socket: The (already open) UDS socket used for communication.
        :param pmc.Config ptp_config: The PTP config to apply to the socket and messages.
        :param int offset_threshold: Values under this offset will be considered as good sync (in ns).
        :param int|None offset_std_threshold: If set, also standard deviation will be checked to be below this (in ns).
        :param int num_offset_values: Length of history.
        :param Callable|None callback: The callback to be called every time something changes. If None, the sync quality
                                       estimate will be printed to stdout.
        :param int subscribe_duration: The duration to be given to SUBSCRIBE_EVENTS_NP (in seconds).
        :param int subscribe_renewal: How often the event subscription should be renewed. This should be lower than
                                      subscribe_duration.
        """
        assert isinstance(socket, pmc.SockUnix)

        self.data = PtpSyncData(num_offset_values, offset_threshold, offset_std_threshold)

        self.sequence = 0
        self.log_level = 1
        self._stop_event = Event()
        self.callback = callback if callback is not None else self.print_status

        self.subscribe_duration = subscribe_duration
        self.subscribe_renewal = subscribe_renewal

        self.ptp_config = ptp_config
        self.socket = socket
        self.message = pmc.Message()
        self.message_buffer = pmc.Buf(1000)
        self.message_lock = Lock()

        self.socket.setDefSelfAddress()
        self.socket.init()
        self.socket.setPeerAddress(self.ptp_config)

        self.message.useConfig(self.ptp_config)

        params = self.message.getParams()
        assert isinstance(params, pmc.MsgParams)
        self_id = params.self_id
        assert isinstance(self_id, pmc.PortIdentity_t)
        self_id.portNumber = os.getpid() & 0xFFFF  # set PID of the current process as port number
        params.self_id = self_id
        params.boundaryHops = 0  # do not query neighbors
        self.message.updateParams(params)

    def __del__(self):
        self._stop_event.set()
        self.print_info("Exiting")

    def is_stopped(self):
        """
        Whether operation of the watchdog should be stopped.
        :return: Whether operation of the watchdog should be stopped.
        :rtype: bool
        """
        return self._stop_event.is_set()

    def print_debug(self, msg):
        """
        Print a debug message.
        :param str msg: The message to print.
        """
        if self.log_level < 1:
            print("DEBUG: " + msg)

    def print_info(self, msg):
        """
        Print an info message.
        :param str msg: The message to print.
        """
        if self.log_level < 2:
            print("INFO:  " + msg)

    def print_warn(self, msg):
        """
        Print a warning message.
        :param str msg: The message to print.
        """
        if self.log_level < 3:
            print("WARN:  " + msg)

    def print_error(self, msg):
        """
        Print an error message.
        :param str msg: The message to print.
        """
        if self.log_level < 4:
            print("ERROR: " + msg)

    def _send_message(self):
        """
        Finish sending self.message via self.socket.
        :raises: RuntimeError If something goes wrong with sending or message building.
        :note: Calling code should already hold self.message_lock.
        """
        if self.is_stopped():
            return

        self.sequence += 1
        err = self.message.build(self.message_buffer, self.sequence)
        if err != pmc.MNG_PARSE_ERROR_OK:
            raise RuntimeError("build error " + pmc.Message.err2str_c(err))
        if not self.socket.send(self.message_buffer, self.message.getMsgLen()):
            raise RuntimeError("Could not send buffer")

    def send_get_message(self, tlv_id):
        """
        Send a GET message TLV.
        :param int tlv_id: The TLV id.
        """
        with self.message_lock:
            self.message.setAction(pmc.GET, tlv_id)
            self._send_message()
        self.print_debug("Sent TLV ID " + str(tlv_id))

    def subscribe_events(self):
        """
        Subscribe to SUBSCRIBE_EVENTS_NP.
        """
        subscribe_message = pmc.SUBSCRIBE_EVENTS_NP_t()
        subscribe_message.duration = self.subscribe_duration
        bitmask_data = get_bitmask_data(subscribe_message)
        bitmask_data[0] = 0 | (1 << pmc.NOTIFY_PORT_STATE) | (1 << pmc.NOTIFY_TIME_SYNC)
        for i in range(1, pmc.EVENT_BITMASK_CNT):
            # noinspection PyTypeChecker
            bitmask_data[i] = 0

        with self.message_lock:
            self.message.setAction(pmc.SET, pmc.SUBSCRIBE_EVENTS_NP, subscribe_message)
            self._send_message()
            self.message.clearData()

        self.print_debug("Subscribed events for another " + str(self.subscribe_duration) + " seconds")

    def receive(self, num_bytes):
        """
        Receive and process data from self.socket after knowing there are some.
        :param int num_bytes: Number of bytes to read.
        :note: The read values are stored in self.data.
        """
        with self.message_lock:
            if pmc.MNG_PARSE_ERROR_OK != self.message.parse(self.message_buffer, num_bytes):
                raise RuntimeError("Could not parse message")

            if not self.check_message_recipient():
                return

            self.print_debug("Received TLV ID: " + str(self.message.getTlvId()))

            if self.message.getTlvId() == pmc.PORT_DATA_SET:
                data = pmc.conv_PORT_DATA_SET(self.message.getData())
                assert isinstance(data, pmc.PORT_DATA_SET_t)
                self.data.port_state = pmc.Message.portState2str_c(data.portState)
            elif self.message.getTlvId() == pmc.PARENT_DATA_SET:
                data = pmc.conv_PARENT_DATA_SET(self.message.getData())
                assert isinstance(data, pmc.PARENT_DATA_SET_t)
                self.data.gm_identity = data.grandmasterIdentity.string()
            elif self.message.getTlvId() == pmc.CURRENT_DATA_SET:
                data = pmc.conv_CURRENT_DATA_SET(self.message.getData())
                assert isinstance(data, pmc.CURRENT_DATA_SET_t)
                self.data.add_offset(data.offsetFromMaster.getIntervalInt())
                self.data.peer_mean_path_delay = data.meanPathDelay.getIntervalInt()
            elif self.message.getTlvId() == pmc.TIME_STATUS_NP:
                data = pmc.conv_TIME_STATUS_NP(self.message.getData())
                assert isinstance(data, pmc.TIME_STATUS_NP_t)
                self.data.add_offset(data.master_offset)
            else:
                self.print_debug("Unknown TLV ID " + str(self.message.getTlvId()))

    def check_message_recipient(self):
        """
        Check the clock and port id to verify the message is for us.
        :return: True if the message should be processed, False otherwise.
        :note: Calling code should already hold self.message_lock.
        """
        peer = self.message.getPeer()
        assert isinstance(peer, pmc.PortIdentity_t)
        clock_id = peer.clockIdentity
        assert isinstance(clock_id, pmc.ClockIdentity_t)
        if self.data.clock_id is None:
            self.data.clock_id = clock_id
            self.print_info("Attached to clock " + clock_id.string())
        elif not clock_id.eq(self.data.clock_id):
            self.print_debug("Received message for a different clock " + clock_id.string())
            return False
        return True

    def watch(self):
        """
        The main watchdog loop.
        """
        self.send_get_message(pmc.PORT_DATA_SET)  # port state
        self.send_get_message(pmc.PARENT_DATA_SET)  # gm identity
        self.send_get_message(pmc.CURRENT_DATA_SET)  # offset, path delay

        self.subscribe_events()  # offset, port state

        threads = [
            SubscriptionRenewalThread(self, self._stop_event),
            CheckPathDelayThread(self, self._stop_event)
        ]

        [t.start() for t in threads]

        while not self.is_stopped():
            try:
                if self.socket.poll(2000):
                    num_bytes = self.socket.rcv(self.message_buffer)
                    self.receive(num_bytes)
                else:
                    self.print_error("Poll failed")
                self.callback()
            except BaseException as e:
                if not isinstance(e, KeyboardInterrupt):
                    self.print_error("Poll error (" + str(type(e)) + "): " + str(e.message))
                if not isinstance(e, Exception):
                    self._stop_event.set()
                    [t.join() for t in threads]

        self._stop_event.set()
        [t.join() for t in threads]

    def print_status(self):
        """
        This is the default callback for events. It prints sync quality messages to stdout.
        """
        if self.data.port_state is not None:
            if self.data.port_state == "SOURCE":
                self.print_info("Port is source clock")
            elif self.data.port_state == "CLIENT":
                quality = self.data.get_sync_quality()
                if quality == PtpSyncQuality.STABLE:
                    self.print_info("Port is synced stable with offset %s from source %r" % (
                        self.data.get_offset_str(), self.data.gm_identity))
                elif quality == PtpSyncQuality.UNSTABLE:
                    self.print_info("Port is synced unstable with offset %s from source %r" % (
                        self.data.get_offset_str(), self.data.gm_identity))
                    self.print_info("Offsets are: " + self.data.get_offsets_str())
                elif quality == PtpSyncQuality.NOT_SYNCED:
                    self.print_warn("Port is NOT in sync yet, current offset %s from source %r" % (
                        self.data.get_offset_str(), self.data.gm_identity))
                    self.print_info("Offsets are: " + self.data.get_offsets_str())
                else:
                    self.print_error("Port sync quality is unknown")

                if self.data.peer_mean_path_delay is not None:
                    self.print_info("Distance to peer %i nanoseconds" % (self.data.peer_mean_path_delay,))
            elif self.data.port_state == "FAULTY":
                self.print_error("Port is faulty")
            else:
                self.print_info("Probing (port state is %s)" % (self.data.port_state,))


def main():
    """
    Main program loop used if you want to call it as a standalone executable.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--ptp-config", default=None, type=str, help="LinuxPTP config file to use.")
    parser.add_argument("-t", "--offset-threshold", default=100, type=int,
                        help="Well synced clocks have offsets lower than this value (in ns).")
    parser.add_argument("-s", "--offset-std-threshold", default=None, type=int,
                        help="Well synced clocks will have standard deviation of last few offsets below this value "
                             "(in ns). If not set, standard deviation is not taken into account.")
    parser.add_argument("-n", "--num-offset-values", default=5, type=int,
                        help="The length of history of offsets to look at.")

    args = parser.parse_args()

    config = pmc.ConfigFile()
    if args.ptp_config is not None:
        print("Using config file " + args.ptp_config)
        config.read_cfg(args.ptp_config)

    with contextlib.closing(pmc.SockUnix()) as sock:
        watchdog = PtpSyncWatchdog(sock, config, args.offset_threshold, args.offset_std_threshold,
                                   args.num_offset_values)
        watchdog.watch()


if __name__ == '__main__':
    main()
