<!--
 *******************************************************************************
  Copyright (c) 2026 Accenture

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# SOME/IP On-Target Testing

Testing against a real S32K148 board connected via a media converter on `enp0s31f6`.

## Network layout

| Side | IP | Notes |
|---|---|---|
| Board (`eth0::IP_ADDRESS`) | `192.168.0.200` | service port `30501`, client port `30502`, SD port `30490` |
| Host (`remoteServiceIp`) | `192.168.0.20` | must match `remoteServiceIp` in `SomeIpSystem.cpp`; assign to `enp0s31f6` |
| Multicast (SD) | `225.0.0.1` | SD port `30490` |

`192.168.0.10` appears in `someip.justfile` for the posix-binary namespace setup and is not used for on-target testing.

## Prerequisites

```bash
pip install someip
```

## Step 1 — configure the host interface

Run once per session (lost on reboot):

```bash
sudo ip addr add 192.168.0.20/24 dev enp0s31f6
sudo ip link set enp0s31f6 up
sudo ip route add 225.0.0.1 dev enp0s31f6
```

## Step 2 — run the Python service

Simulates the remote ECU (`0xBABE`) that the board subscribes to. The board expects it at `192.168.0.20:30502`.

```bash
python3 tools/someip/service.py \
    --serviceid=47806 \
    --ip=192.168.0.20 \
    --multicast=225.0.0.1 \
    --port=30502
```

## Expected Wireshark traffic after Step 2

| Direction | Src → Dst | Port | What it is |
|---|---|---|---|
| Host → multicast | `192.168.0.20` → `225.0.0.1` | `30490` | SD **Offer** for service `0xBABE` |
| Board → host | `192.168.0.200` → `192.168.0.20` | `30490` | SD **Subscribe** for eventgroup `0x8001` |
| Host → board | `192.168.0.20` → `192.168.0.200` | `30490` | SD **SubscribeAck** |
| Host → board | `192.168.0.20` → `192.168.0.200` | `30502` | **Event notifications** (method_id `0x8001`, ~34 bytes, periodic) |

The ~34-byte unicast packets on port `30502` are event notifications carrying the clock timestamp — **not** SD Offers. Their presence means the full subscription flow succeeded.

## Step 3 — send a request to the board's service

In a second terminal. Sends payload `0xCAFEBABE` to service `0xCAFE` on the board and expects `0xBABECAFE` back (firmware swaps the 16-bit halves).

```bash
python3 tools/someip/client.py \
    --ifver=1 \
    --serviceid=51966 \
    --ip=192.168.0.200 \
    --port=30501
```

## Cleanup

```bash
sudo ip addr del 192.168.0.20/24 dev enp0s31f6
sudo ip route del 225.0.0.1 dev enp0s31f6
```
