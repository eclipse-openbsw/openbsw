# *******************************************************************************
# Copyright (c) 2026 KPIT Group
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""SOME/IP helpers for testing the referenceApp's middleware SOME/IP bridge sample.

The sample provides instance 1 of service 0x0101 from the FLYNC model on UDP port
30501 and consumes instance 2. SampleClient uses instance 1 the way a remote
consumer would; Peer provides instance 2 the way the peer node would. Both need
only the Python standard library.
"""

import socket
import struct
import threading
import time

SERVICE_ID = 0x0101
MAJOR_VERSION = 1
MINOR_VERSION = 1
SAMPLE_INSTANCE = 1
PEER_INSTANCE = 2
SERVICE_PORT = 30501
EVENTGROUP_ALL = 0x0002
EVENT_MULTICAST = ("239.2.41.16", SERVICE_PORT)

SD_PORT = 30490
SD_MULTICAST = "225.0.0.1"
SD_SERVICE_ID = 0xFFFF
SD_METHOD_ID = 0x8100

ECHO_UINT8 = 0x0008
GET_TEST_FIELD_UINT8 = 0x0026
SET_TEST_FIELD_UINT8 = 0x0027
TEST_FIELD_UINT8_NOTIFIER = 0x8006
TEST_EVENT_UINT8_MULTICAST = 0x800B

REQUEST = 0x00
NOTIFICATION = 0x02
RESPONSE = 0x80
ERROR = 0x81

FIND_SERVICE = 0x00
OFFER_SERVICE = 0x01
SUBSCRIBE_EVENTGROUP = 0x06
SUBSCRIBE_EVENTGROUP_ACK = 0x07
IPV4_ENDPOINT_OPTION = 0x04
IPV4_MULTICAST_OPTION = 0x14
UDP = 0x11

TTL_S = 10

HEADER = struct.Struct("!HHIHHBBBB")


def local_ip_for(target_ip):
    """This host's address on the route to the target."""
    probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        probe.connect((target_ip, 9))
        return probe.getsockname()[0]
    finally:
        probe.close()


def build(service, method, session, message_type, payload=b"", client=0x0001):
    """A SOME/IP message: the 16-byte header, then the payload."""
    return (
        HEADER.pack(
            service,
            method,
            8 + len(payload),
            client,
            session,
            1,
            MAJOR_VERSION,
            message_type,
            0,
        )
        + payload
    )


def parse(data):
    """(service, method, client, session, message type, return code, payload)."""
    service, method, _, client, session, _, _, message_type, return_code = (
        HEADER.unpack_from(data)
    )
    return service, method, client, session, message_type, return_code, data[HEADER.size :]


def sd_message(session, entries, options):
    """A SOME/IP-SD message with the reboot and unicast flags set."""
    entries = b"".join(entries)
    options = b"".join(options)
    payload = (
        bytes([0xC0, 0, 0, 0])
        + struct.pack("!I", len(entries))
        + entries
        + struct.pack("!I", len(options))
        + options
    )
    return build(SD_SERVICE_ID, SD_METHOD_ID, session, NOTIFICATION, payload, client=0)


def ipv4_option(option_type, ip, port):
    return struct.pack(
        "!HBB4sBBH", 9, option_type, 0, socket.inet_aton(ip), 0, UDP, port
    )


def offer_entry(instance):
    return struct.pack(
        "!BBBBHHB3sI",
        OFFER_SERVICE,
        0,
        0,
        0x10,
        SERVICE_ID,
        instance,
        MAJOR_VERSION,
        TTL_S.to_bytes(3, "big"),
        MINOR_VERSION,
    )


def eventgroup_entry(entry_type, instance, eventgroup, ttl):
    return struct.pack(
        "!BBBBHHB3sHH",
        entry_type,
        0,
        0,
        0x10,
        SERVICE_ID,
        instance,
        MAJOR_VERSION,
        ttl,
        0,
        eventgroup,
    )


def sd_entries(body):
    """Yield (type, service, instance, ttl, eventgroup, endpoint) per SD entry."""
    (entries_length,) = struct.unpack_from("!I", body, 4)
    entries = body[8 : 8 + entries_length]
    (options_length,) = struct.unpack_from("!I", body, 8 + entries_length)
    raw_options = body[12 + entries_length : 12 + entries_length + options_length]
    options = []
    offset = 0
    while offset + 3 <= len(raw_options):
        (length,) = struct.unpack_from("!H", raw_options, offset)
        option_type = raw_options[offset + 2]
        if option_type in (IPV4_ENDPOINT_OPTION, IPV4_MULTICAST_OPTION) and length == 9:
            ip = socket.inet_ntoa(raw_options[offset + 4 : offset + 8])
            (port,) = struct.unpack_from("!H", raw_options, offset + 10)
            options.append((ip, port))
        else:
            options.append(None)
        offset += 3 + length
    for offset in range(0, len(entries), 16):
        entry = entries[offset : offset + 16]
        entry_type, first_option, _, option_counts, service, instance = struct.unpack_from(
            "!BBBBHH", entry
        )
        ttl = entry[9:12]
        endpoint = None
        if (option_counts >> 4) and first_option < len(options):
            endpoint = options[first_option]
        eventgroup = None
        if entry_type in (SUBSCRIBE_EVENTGROUP, SUBSCRIBE_EVENTGROUP_ACK):
            (eventgroup,) = struct.unpack_from("!H", entry, 14)
        yield entry_type, service, instance, ttl, eventgroup, endpoint


class SampleClient:
    """Uses the sample's instance 1: calls its method and field, subscribes to eg2."""

    def __init__(self, target_ip):
        self.target_ip = target_ip
        self.target = (target_ip, SERVICE_PORT)
        self.local_ip = local_ip_for(target_ip)
        self._session = 0
        self._sd = None
        self._multicast = None
        self.rpc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.rpc.bind((self.local_ip, 0))
        self.rpc.settimeout(1.0)

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def close(self):
        for sock in (self.rpc, self._sd, self._multicast):
            if sock is not None:
                sock.close()

    def _next_session(self):
        self._session = self._session % 0xFFFF + 1
        return self._session

    def send(self, method, payload=b""):
        """Send one request without waiting for its response."""
        self.rpc.sendto(
            build(SERVICE_ID, method, self._next_session(), REQUEST, payload), self.target
        )

    def call(self, method, payload=b"", attempts=3):
        """Send one request and return the payload of its response.

        UDP gives no delivery guarantee, and the first datagram to a host can be
        lost while its address is resolved, so the request is sent again on a
        timeout.
        """
        for _ in range(attempts):
            session = self._next_session()
            self.rpc.sendto(build(SERVICE_ID, method, session, REQUEST, payload), self.target)
            try:
                while True:
                    data, _ = self.rpc.recvfrom(1500)
                    service, answered, _, answered_session, message_type, return_code, body = (
                        parse(data)
                    )
                    if (service, answered, answered_session) != (SERVICE_ID, method, session):
                        continue
                    if message_type == ERROR or return_code != 0:
                        raise RuntimeError(
                            f"method {method:#06x} answered with error {return_code:#04x}"
                        )
                    if message_type == RESPONSE:
                        return body
            except socket.timeout:
                continue
        raise RuntimeError(f"no response to method {method:#06x}")

    def subscribe(self, timeout=2.0):
        """Subscribe to eg2 and join its multicast group; True once acknowledged."""
        self._sd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sd.bind((self.local_ip, SD_PORT))
        self._sd.settimeout(0.2)

        self._multicast = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._multicast.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._multicast.bind(("", EVENT_MULTICAST[1]))
        self._multicast.setsockopt(
            socket.IPPROTO_IP,
            socket.IP_ADD_MEMBERSHIP,
            socket.inet_aton(EVENT_MULTICAST[0]) + socket.inet_aton(self.local_ip),
        )
        self._multicast.setblocking(False)

        entry = eventgroup_entry(
            SUBSCRIBE_EVENTGROUP, SAMPLE_INSTANCE, EVENTGROUP_ALL, TTL_S.to_bytes(3, "big")
        )
        option = ipv4_option(IPV4_ENDPOINT_OPTION, self.local_ip, self.rpc.getsockname()[1])
        self._sd.sendto(
            sd_message(self._next_session(), [entry], [option]), (self.target_ip, SD_PORT)
        )
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                data = self._sd.recv(1500)
            except socket.timeout:
                continue
            for entry_type, service, instance, _, eventgroup, _ in sd_entries(data[HEADER.size :]):
                if (entry_type, service, instance, eventgroup) == (
                    SUBSCRIBE_EVENTGROUP_ACK,
                    SERVICE_ID,
                    SAMPLE_INSTANCE,
                    EVENTGROUP_ALL,
                ):
                    return True
        return False

    def collect(self, duration):
        """Return the (method, value) notifications that arrive within duration."""
        received = []
        self.rpc.setblocking(False)
        try:
            end = time.monotonic() + duration
            while time.monotonic() < end:
                for sock in (self.rpc, self._multicast):
                    if sock is None:
                        continue
                    try:
                        data = sock.recv(1500)
                    except BlockingIOError:
                        continue
                    service, method, _, _, message_type, _, body = parse(data)
                    if service == SERVICE_ID and message_type == NOTIFICATION and body:
                        received.append((method, body[0]))
                time.sleep(0.01)
        finally:
            self.rpc.settimeout(1.0)
        return received


class Peer(threading.Thread):
    """Provides instance 2 for the sample to consume, the way the peer node would.

    It offers the instance through SD, acknowledges the subscription to eg2 with
    the multicast group, answers echoUINT8 and the TestFieldUINT8 getter and
    setter, notifies TestFieldUINT8 on a set, and publishes TestEventUINT8Multicast
    once a second.
    """

    FIRST_EVENT = 101

    def __init__(self, target_ip, field=7):
        super().__init__(daemon=True)
        self.target_ip = target_ip
        self.local_ip = local_ip_for(target_ip)
        self.field = field
        self.subscribed = False
        self.requests = []
        self._session = 0
        self._stop_event = threading.Event()

    def stop(self):
        self._stop_event.set()
        self.join(timeout=2.0)

    def _next_session(self):
        self._session = self._session % 0xFFFF + 1
        return self._session

    def run(self):
        sd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sd.bind(("", SD_PORT))
        sd.setsockopt(
            socket.IPPROTO_IP,
            socket.IP_ADD_MEMBERSHIP,
            socket.inet_aton(SD_MULTICAST) + socket.inet_aton(self.local_ip),
        )
        sd.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(self.local_ip))
        sd.setblocking(False)

        service = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        service.bind((self.local_ip, SERVICE_PORT))
        service.setsockopt(
            socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(self.local_ip)
        )
        service.setblocking(False)

        subscriber = None
        counter = self.FIRST_EVENT - 1
        next_offer = 0.0
        next_event = 0.0
        try:
            while not self._stop_event.is_set():
                now = time.monotonic()
                if now >= next_offer:
                    offer = sd_message(
                        self._next_session(),
                        [offer_entry(PEER_INSTANCE)],
                        [ipv4_option(IPV4_ENDPOINT_OPTION, self.local_ip, SERVICE_PORT)],
                    )
                    sd.sendto(offer, (SD_MULTICAST, SD_PORT))
                    next_offer = now + 1.0

                subscriber = self._handle_sd(sd, subscriber)
                self._handle_request(service, subscriber)

                if subscriber is not None and now >= next_event:
                    counter = (counter + 1) % 256
                    service.sendto(
                        build(SERVICE_ID, TEST_EVENT_UINT8_MULTICAST, 0, NOTIFICATION, bytes([counter]), 0),
                        EVENT_MULTICAST,
                    )
                    next_event = now + 1.0
                time.sleep(0.005)
        finally:
            sd.close()
            service.close()

    def _handle_sd(self, sd, subscriber):
        try:
            data, address = sd.recvfrom(1500)
        except BlockingIOError:
            return subscriber
        if address[0] != self.target_ip:
            return subscriber
        for entry_type, service_id, instance, ttl, eventgroup, endpoint in sd_entries(
            data[HEADER.size :]
        ):
            if (entry_type, service_id, instance) != (SUBSCRIBE_EVENTGROUP, SERVICE_ID, PEER_INSTANCE):
                continue
            ack = sd_message(
                self._next_session(),
                [eventgroup_entry(SUBSCRIBE_EVENTGROUP_ACK, PEER_INSTANCE, eventgroup, ttl)],
                [ipv4_option(IPV4_MULTICAST_OPTION, *EVENT_MULTICAST)],
            )
            sd.sendto(ack, (self.target_ip, SD_PORT))
            self.subscribed = True
            subscriber = endpoint
        return subscriber

    def _handle_request(self, service, subscriber):
        try:
            data, address = service.recvfrom(1500)
        except BlockingIOError:
            return
        service_id, method, client, session, message_type, _, body = parse(data)
        if service_id != SERVICE_ID or message_type != REQUEST:
            return
        self.requests.append(method)
        if method == ECHO_UINT8:
            reply = body[:1]
        elif method == GET_TEST_FIELD_UINT8:
            reply = bytes([self.field])
        elif method == SET_TEST_FIELD_UINT8:
            self.field = body[0]
            reply = bytes([self.field])
            if subscriber is not None:
                service.sendto(
                    build(SERVICE_ID, TEST_FIELD_UINT8_NOTIFIER, 0, NOTIFICATION, reply, 0),
                    subscriber,
                )
        else:
            return
        service.sendto(build(SERVICE_ID, method, session, RESPONSE, reply, client), address)
