# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import asyncio
import datetime
import socket
import logging
import argparse

from someip.header import _T_SOCKNAME, SOMEIPHeader
from someip.sd import ServiceDiscoveryProtocol
from someip.service import MalformedMessageError, SimpleEventgroup
from someip.service import SimpleService

LOG = logging.getLogger("someip-service")


class TimeEvents(SimpleEventgroup):
    def __init__(self, service):
        super().__init__(service, id=0x8001, interval=1)
        asyncio.create_task(self._tick())

    async def _tick(self):
        while True:
            self.values[0x0001] = datetime.datetime.now().isoformat().encode()
            await asyncio.sleep(0.9)


class ClockService(SimpleService):
    service_id = 0xBABE
    version_major = 1
    version_minor = 0

    def __init__(self, instance_id: int):
        super().__init__(instance_id)
        self.register_method(1, self.get_time)
        self.register_eventgroup(TimeEvents(self))
        self._offset = datetime.timedelta()

    def get_time(self, msg: SOMEIPHeader, _addr: _T_SOCKNAME) -> bytes:
        if msg.payload:
            raise MalformedMessageError
        return (datetime.datetime.now() + self._offset).isoformat().encode("ascii")


def setup_log(fmt="", **kwargs):
    logging.basicConfig(format="%(asctime)s " + fmt, **kwargs)


async def main():
    setup_log("%(levelname)-8s %(name)s: %(message)s", level=logging.INFO)

    parser = argparse.ArgumentParser()
    parser.add_argument("--serviceid", type=int)
    parser.add_argument("--port", type=int)
    parser.add_argument("--ip", type=str)
    parser.add_argument("--multicast", type=str)
    args = parser.parse_args()

    sd_u, sd_m, sd = await ServiceDiscoveryProtocol.create_endpoints(
        family=socket.AF_INET,
        local_addr=args.ip,
        multicast_addr=args.multicast,
    )

    ClockService.service_id = args.serviceid
    clock = await ClockService.start_datagram_endpoint(
        instance_id=1,
        announcer=sd.announcer,
        local_addr=(args.ip, args.port),
    )

    sd.start()
    try:
        await asyncio.Event().wait()
    finally:
        sd.stop()
        sd_u.close()
        sd_m.close()
        clock.stop()


"""
    Example command:
    python service.py \
        --serviceid=47806 \
        --port=30501 \
        --ip=192.168.0.20
        --multicast=225.0.0.1
"""

if __name__ == "__main__":
    asyncio.run(main())
