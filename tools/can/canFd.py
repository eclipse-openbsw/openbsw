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

"""
canFd.py - CAN FD test sender for OpenBSW/FlexCAN

Sends CAN FD frames with lengths 8, 12, 16, 32, 64 bytes on a SocketCAN
interface (e.g. vcan0), with optional BRS and simple receive echo.

Examples:
  Classical FD (no BRS):
    python3 ./canFd.py -i vcan0

  With BRS:
    python3 ./canFd.py -i vcan0 --brs

  Single length only (e.g. 64 bytes):
    python3 ./canFd.py -i vcan0 --lengths 64

  Run with echo listen:
    python3 ./canFd.py -i vcan0 --listen
"""

import argparse
import time
from typing import List

import can  # pip install python-can


def make_payload(length: int) -> bytes:
    """Generate deterministic test payload 0x00..0xNN."""
    return bytes((i & 0xFF) for i in range(length))


def send_frames(
    bus: can.BusABC,
    arb_id: int,
    lengths: List[int],
    brs: bool,
    listen: bool,
    delay_s: float,
) -> None:
    for idx, length in enumerate(lengths, start=1):
        data = make_payload(length)
        msg = can.Message(
            arbitration_id=arb_id,
            is_extended_id=False,
            is_fd=True,
            bitrate_switch=brs,
            data=data,
        )

        print(f"[{idx}] Sending FD frame: ID=0x{arb_id:X}, len={len(data)}, BRS={msg.bitrate_switch}")
        print("     Payload:", " ".join(f"{b:02X}" for b in data))

        try:
            bus.send(msg)
            print("     -> send OK")
        except can.CanError as e:
            print(f"     -> send FAILED: {e}")
            continue

        if listen:
            print("     Waiting up to 0.5s for frame on this socket...")
            rx = bus.recv(timeout=0.5)
            if rx is None:
                print("     -> no frame received on this socket")
            else:
                print(
                    f"     -> received: ID=0x{rx.arbitration_id:X}, "
                    f"len={len(rx.data)}, FD={rx.is_fd}, BRS={rx.bitrate_switch}"
                )
                print("        Data:", " ".join(f"{b:02X}" for b in rx.data))

        time.sleep(delay_s)


def parse_lengths(arg: str) -> List[int]:
    if not arg:
        # Default lengths: 8, 12, 16, 32, 64
        return [8, 12, 16, 32, 64]
    out: List[int] = []
    for part in arg.split(","):
        v = int(part.strip())
        if v <= 0 or v > 64:
            raise ValueError(f"Invalid length {v}, must be 1..64")
        out.append(v)
    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--interface", default="vcan0",
                        help="SocketCAN interface (default: vcan0)")
    parser.add_argument("-a", "--arbid", default="0x123",
                        help="Arbitration ID in hex (default: 0x123)")
    parser.add_argument("--brs", action="store_true",
                        help="Enable bitrate switch (BRS) on FD frames")
    parser.add_argument("--lengths", default="",
                        help="Comma-separated payload lengths, e.g. '8,12,16,32,64' "
                             "(default: 8,12,16,32,64)")
    parser.add_argument("--listen", action="store_true",
                        help="Listen for frames on same socket after each send")
    parser.add_argument("--delay", type=float, default=0.1,
                        help="Delay between frames in seconds (default: 0.1)")
    args = parser.parse_args()

    arb_id = int(args.arbid, 16)
    lengths = parse_lengths(args.lengths)

    print(f"Opening SocketCAN FD bus on {args.interface}...")
    bus = can.interface.Bus(
        channel=args.interface,
        interface="socketcan",
        fd=True,                # required for CAN FD[web:683]
        receive_own_messages=True,
    )
    print(f"Bus opened: {bus}")
    print(f"Bus state: {bus.state}")

    try:
        send_frames(
            bus=bus,
            arb_id=arb_id,
            lengths=lengths,
            brs=args.brs,
            listen=args.listen,
            delay_s=args.delay,
        )
    finally:
        try:
            bus.shutdown()
        except Exception:
            pass


if __name__ == "__main__":
    main()
