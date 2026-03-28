---
title: CAN Echo Test — CVC→RZC Fire-and-Forget
status: PENDING
date: 2026-03-28
---

# CAN Echo Test: CVC (FreeRTOS) → RZC (ThreadX)

## Goal

Prove that fire-and-forget CAN communication works 100% between two OpenBSW
G474RE boards running different RTOSes. CVC sends a frame every 100ms,
RZC counts what it receives, Pi monitors the bus.

## Setup

- **CVC** (SN 0027, COM7): OpenBSW FreeRTOS — sends 0x111 every 100ms with counter
- **RZC** (SN 0049, COM8): OpenBSW ThreadX — receives 0x111, prints count every 5s on UART
- **Pi** (can0): candump monitors bus, counts frames
- **CH340** (COM17): independent sniffer (optional)

## What We Test

1. CVC fire-and-forget TX reliability (does every frame reach the bus?)
2. RZC RX reliability (does the board receive every frame?)
3. Pi RX reliability (does the Pi see every frame?)
4. Cross-RTOS interop (FreeRTOS TX → ThreadX RX)

## Implementation

### Step 1: Add echo sender to CVC CanSystem

In `CanSystem::run()`, schedule a periodic runnable at 100ms that sends:
```
CAN ID: 0x111
DLC: 8
Data: [counter_hi, counter_lo, 0xAA, 0xBB, 0x00, 0x00, 0x00, 0x00]
```

Counter increments each send. Use `write(frame)` — fire-and-forget, no listener.

### Step 2: Add echo receiver to RZC CanSystem

Register a CAN listener for ID 0x111. On receive:
- Increment rx_counter
- Store last received counter value from payload

Every 5s, print one short line to UART:
```
E rx=<count> last=<counter> miss=<expected-count>
```

### Step 3: Test script on Pi

```bash
# Run for 30s
timeout 30 candump can0 -t d | tee /tmp/echo.log
# Count
grep '111' /tmp/echo.log | wc -l   # should be ~300 (30s × 10/s)
grep '558' /tmp/echo.log | wc -l   # demo frames still flowing
```

### Step 4: Compare

| Source | Expected (30s) | Metric |
|--------|----------------|--------|
| CVC UART | tx=300 | frames queued to FDCAN |
| candump 0x111 | ~300 | frames on the bus |
| RZC UART | rx=~300 | frames received by RZC |
| Counter gaps | 0 | no missed sequence numbers |

## Files to Modify

- `executables/referenceApp/platforms/nucleo_g474re/main/src/systems/CanSystem.cpp` — add echo sender (CVC) / echo receiver (RZC)
- `executables/referenceApp/platforms/nucleo_g474re/main/include/systems/CanSystem.h` — add runnable + listener declarations

Since both CVC and RZC use the same CanSystem source, use a `#define` or runtime
check to switch between sender and receiver mode. Simplest: use a different CAN ID
for the echo test and add both sender and receiver to the same code. CVC sends on
0x111, RZC listens on 0x111. Both boards run the same binary — the behavior is
identical (both send AND receive 0x111). Compare TX count vs RX count on each board.

## Build & Flash

```bash
# Build both from D:\openbsw
cmake --build --preset nucleo-g474re-freertos-gcc   # CVC
cmake --build --preset nucleo-g474re-threadx-gcc    # RZC

# Flash
st-flash --serial 0027... erase && st-flash --serial 0027... --reset write cvc.bin 0x08000000
st-flash --serial 0049... erase && st-flash --serial 0049... --reset write rzc.bin 0x08000000
```

## Success Criteria

- 0x111 frame delivery ≥ 99% in both directions
- No sequence number gaps
- Both RTOS builds show identical behavior
