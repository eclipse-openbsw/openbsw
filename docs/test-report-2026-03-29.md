# Test Report — G4 FdCanDevice S32K Contract Match

**Date:** 2026-03-29
**Branch:** stm32-platform-port
**Build host:** an-dao@192.168.0.158 (Ubuntu x86_64)

## Summary

| Suite | Tests | Passed | Failed |
|-------|-------|--------|--------|
| STM32 (tests-stm32-debug) | 152 | 152 | 0 |
| POSIX (tests-posix-debug) | 1470 | 1470 | 0 |
| **Total** | **1622** | **1622** | **0** |

## STM32 Test Breakdown

| Executable | Tests | Status |
|-----------|-------|--------|
| FdCanTransceiverTest | 71 | ALL PASS |
| BxCanTransceiverTest | 80 | ALL PASS |
| bspUartTest | 1 | ALL PASS |

## POSIX Test Breakdown

| Executable | Tests |
|-----------|-------|
| asyncFreeRtosTest | 41 |
| asyncImplTest | 9 |
| bspCharInputOutputTest | 5 |
| bspDynamicClientTest | 4 |
| bspInputManagerTest | 5 |
| bspInterruptsTest | 2 |
| bspOutputManagerTest | 1 |
| bspOutputPwmTest | 1 |
| bspTest | 7 |
| cpp2canTest | 33 |
| cpp2ethernetTest | 47 |
| docanTest | 168 |
| doipTest | 160 |
| ioTest | 218 |
| lifecycleTest | 12 |
| loggerTest | 42 |
| middlewareTest | 102 |
| platformTest | 1 |
| runtimeTest | 32 |
| safeMonitorTest | 42 |
| socketCanTransceiverTest | 5 |
| storageTest | 23 |
| timerTest | 36 |
| transportTest | 29 |
| udsTest | 275 |
| utilTest | 185 |

## Changes Under Test

- FdCanDevice: callback delegate (match FlexCANDevice), selective TCE, RF0NE disable in receiveISR
- FdCanTransceiver: uses new device API, simplified transmitInterrupt
- CanSystem: removed defensive IE restore, IE=RF0NE only at init
- Mock FdCanDevice: stores and invokes callback delegate
