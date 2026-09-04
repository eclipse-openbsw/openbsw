#!/usr/bin/env python3
# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import argparse
import asyncio
import logging
import struct

import someip.header
from someip.sd import SOMEIPDatagramProtocol

ECHO_UINT32_METHOD_ID = 1

ECHO_UINT32 = 0xBABECAFE

LOG = logging.getLogger("someip-client")


class SOMEIP_Client(SOMEIPDatagramProtocol):
    def handle_echo_uint32(self, someip_message: someip.header.SOMEIPHeader) -> None:
        (u32,) = struct.unpack("!L", someip_message.payload)
        if ECHO_UINT32 == u32:
            LOG.info(f"correct, value - {hex(u32)}")
        else:
            LOG.info(f"incorrect, value - {hex(u32)}, expected - {hex(ECHO_UINT32)}")

    def message_received(
        self,
        someip_message: someip.header.SOMEIPHeader,
        addr: someip.header._T_SOCKNAME,
        multicast: bool,
    ) -> None:
        if someip_message.method_id == ECHO_UINT32_METHOD_ID:
            self.handle_echo_uint32(someip_message)


def echo_uint32(ets, ifver, serviceid):
    hdr = someip.header.SOMEIPHeader(
        service_id=serviceid,
        method_id=ECHO_UINT32_METHOD_ID,
        client_id=0,
        session_id=0,
        interface_version=ifver,
        message_type=someip.header.SOMEIPMessageType.REQUEST,
        payload=struct.pack("!L", 0xCAFEBABE),
    )

    ets.send(hdr.build())


async def testing_session(args):
    transport, ets = await SOMEIP_Client.create_unicast_endpoint(
        remote_addr=(args.ip, args.port), local_addr=("0.0.0.0", args.port)
    )
    try:
        echo_uint32(ets, args.ifver, args.serviceid)
        await asyncio.sleep(args.interval)
    except asyncio.CancelledError:
        pass
    finally:
        transport.close()


def setup_log(fmt="", **kwargs):
    logging.basicConfig(format="%(asctime)s " + fmt, **kwargs)


def main():
    setup_log("%(levelname)-8s %(name)s: %(message)s", level=logging.INFO)

    parser = argparse.ArgumentParser()
    parser.add_argument("--serviceid", type=int)
    parser.add_argument("--ifver", type=int)
    parser.add_argument("--ip", type=str)
    parser.add_argument("--port", type=int)
    parser.add_argument("--interval", type=int, default=3)
    args = parser.parse_args()

    try:
        asyncio.run(testing_session(args))
        # TODO: regexp to run specific test / wildcard
    except KeyboardInterrupt:
        pass


"""
    Example command:
    python client.py \
        --ifver=1 \
        --serviceid=51966 \
        --ip=192.168.0.201 \
        --port=30501
"""
if __name__ == "__main__":
    main()
