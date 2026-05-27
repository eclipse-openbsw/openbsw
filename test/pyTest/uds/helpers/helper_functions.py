# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import binascii
import udsoncan
import subprocess
import time
import select
import os


class ByteCodec(udsoncan.DidCodec):
    def encode(self, value: bytes) -> bytes:
        return value

    def decode(self, data: bytes) -> bytes:
        return data


def hexlify(value):
    response = binascii.hexlify(value).decode("ascii")
    formatted_response = " ".join(
        response[i : i + 2] for i in range(0, len(response), 2)
    )
    return formatted_response


def run_process(cmd, output_contains, timeout=5):
    """Run a shell command and wait until specific output appears.

    The command is executed in ``/bin/bash`` using ``subprocess``. The function waits
    until `` output_contains`` is matched in the process output and returns the captured
    text up to and including the matched token, plus the remainder of that line
    (if any).

    Args:
        cmd: Shell command to execute.
        output_contains: Output text or pattern to wait for.
        timeout: Maximum number of seconds to wait for `` output_contains``.

    Returns:
        Captured process output containing `` output_contains``.

    Raises:
        AssertionError: If the process times out or exits before producing
            `` output_contains``.
    """
    proc = subprocess.Popen(
        ["/bin/bash", "-c", cmd],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=False,
        bufsize=0,
    )

    poller = select.poll()
    poller.register(proc.stdout, select.POLLIN)

    output = []
    deadline = time.monotonic() + timeout
    try:
        while True:
            remaining_ms = max(0, int((deadline - time.monotonic()) * 1000))
            events = poller.poll(remaining_ms)
            if remaining_ms <= 0:
                raise subprocess.TimeoutExpired(cmd=cmd, timeout=timeout) 
            if not events:
                continue                  
            chunk = os.read(proc.stdout.fileno(), 4096).decode()
            if not chunk:
                if proc.poll() is not None:
                    break
            output.append(chunk)
            combined = "".join(output)

            if output_contains in combined:
                return combined
        raise AssertionError(f"Did not find {output_contains} within {timeout}s\n Output:\n{''.join(output)}")
    finally:
        proc.kill()
        proc.wait()
