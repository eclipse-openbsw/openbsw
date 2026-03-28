# STM32 Platform Port — Fix & Verification Task Plan

**Branch:** `stm32-platform-port`
**Generated:** 2026-03-27 (revised after live verification of all file states)
**Input:** `docs/stm32-bsp-findings.md`, `docs/stm32-app-findings.md`, `docs/stm32-port-verification.md`
**Workers:** 3 Sonnet agents (W1, W2, W3)
**Constraint:** Each task ≤ 10 minutes. No `git push`, no `rm -rf`, no `flash`.

---

## Pre-flight: What was verified

All findings were confirmed still present (no fixes applied yet):
- FdCanTransceiver: `close()` and `mute()` still missing `fTxQueue.clear()`; `canFrameSentAsyncCallback()` still has `State::INITIALIZED` check
- FdCanDevice.h: internal methods still `public` at line 186
- clockConfig_f4.cpp: no flash latency readback loop after line 44
- Both linker scripts: `_Min_Stack_Size = 0x400`, no ASSERT
- Both FreeRTOS configs (F413ZH AND G474RE): `configUSE_MALLOC_FAILED_HOOK = 0` at line 33
- CLK-02 (SystemInit): **NOT a bug** — `SystemInit()` IS defined in both `main.cpp` files (F413ZH line 43, G474RE line 24)

---

## Task Overview

| ID | Worker | Est. | Summary |
|----|--------|------|---------|
| T1 | W1 | 8 min | Fix FdCanTransceiver bugs (CAN-01, CAN-02, CAN-03) |
| T2 | W2 | 6 min | Fix FdCanDevice encapsulation + Fix FreeRTOS malloc hook (DEV-01, F-01) |
| T3 | W3 | 7 min | Fix linker scripts: stack size + overflow guard (LD-01, LD-03) |
| T4 | W1 | 5 min | Fix clockConfig_f4 flash latency readback (CLK-03) |
| T5 | W3 | 8 min | Verify ALL fixes from T1–T4 and T2; write verification report |

---

## T1 — Fix FdCanTransceiver Bugs (W1, ~8 min)

**Findings:** CAN-01, CAN-02, CAN-03

### Step 1: Read these files
1. `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp` — full file
2. `platforms/stm32/bsp/bxCanTransceiver/src/can/transceiver/bxcan/BxCanTransceiver.cpp` — lines 86–116 and 270–285 only (reference)

### Step 2: Make exactly 3 edits in FdCanTransceiver.cpp

**Edit 1 — CAN-01: Add `fTxQueue.clear()` in `close()` (between lines 93–94)**

Current (lines 93–94):
```cpp
    fDevice.stop();
    setState(State::CLOSED);
```

Change to:
```cpp
    fDevice.stop();
    fTxQueue.clear();
    setState(State::CLOSED);
```

**Edit 2 — CAN-02: Add `fTxQueue.clear()` in `mute()` (between lines 107–108)**

Current (lines 107–108):
```cpp
    fMuted = true;
    setState(State::MUTED);
```

Change to:
```cpp
    fMuted = true;
    fTxQueue.clear();
    setState(State::MUTED);
```

**Edit 3 — CAN-03: Remove `State::INITIALIZED` from guard (line 224)**

Current (line 224):
```cpp
                if ((State::OPEN == state) || (State::INITIALIZED == state))
```

Change to:
```cpp
                if (State::OPEN == state)
```

### Step 3: Verify
- Read the final `FdCanTransceiver.cpp` to confirm all 3 edits applied
- `Grep` for `INITIALIZED` in the file — should appear only in `init()` function, NOT in `canFrameSentAsyncCallback()`
- Compare `close()` and `mute()` structure against `BxCanTransceiver.cpp` lines 90–116

---

## T2 — Fix FdCanDevice Encapsulation + FreeRTOS Malloc Hook (W2, ~6 min)

**Findings:** DEV-01, F-01

### Part A: FdCanDevice encapsulation (DEV-01)

#### Step 1: Read these files
1. `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h` — full file (focus lines 186–195)
2. `platforms/stm32/bsp/bspCan/include/can/BxCanDevice.h` — lines 206–218 (reference: these are private)

#### Step 2: Edit FdCanDevice.h

Change lines 186–195 from:
```cpp
public:
    bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

    void enterInitMode();
    void leaveInitMode();
    void configureBitTiming();
    void configureMessageRam();
    void configureGpio();
    void enablePeripheralClock();
};
```

To:
```cpp
    bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

private:
    void enterInitMode();
    void leaveInitMode();
    void configureBitTiming();
    void configureMessageRam();
    void configureGpio();
    void enablePeripheralClock();
};
```

**IMPORTANT:** `fTxEventEnabled` MUST remain public — it is accessed directly from `FdCanTransceiver.cpp` (lines 163, 166, 170, 241, 246). Only move the 6 methods below it to `private:`. Remove the `public:` label before `fTxEventEnabled` (it inherits the prior access specifier — check what that is. If the prior section is `private`, add a temporary `public:` for `fTxEventEnabled` only).

#### Step 3: Verify encapsulation
- `Grep` for `enterInitMode|leaveInitMode|configureBitTiming|configureMessageRam|configureGpio|enablePeripheralClock` in `platforms/stm32/bsp/bspCan/src/can/FdCanDevice.cpp` — confirm these are only called from within the class itself
- Read `platforms/stm32/bsp/fdCanTransceiver/test/mock/include/can/FdCanDevice.h` — note the mock's access specifiers (line ~55–62)

### Part B: FreeRTOS malloc hook (F-01) — BOTH platforms

#### Step 4: Read these 4 files
1. `executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` — line 33
2. `executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` — line 33
3. `executables/referenceApp/platforms/nucleo_f413zh/main/src/osHooks/freertos/osHooks.cpp` — confirm `vApplicationMallocFailedHook` exists
4. `executables/referenceApp/platforms/nucleo_g474re/main/src/osHooks/freertos/osHooks.cpp` — confirm `vApplicationMallocFailedHook` exists

#### Step 5: Edit BOTH FreeRtosPlatformConfig.h files

In BOTH files, change line 33:
```cpp
#define configUSE_MALLOC_FAILED_HOOK            0
```
to:
```cpp
#define configUSE_MALLOC_FAILED_HOOK            1
```

#### Step 6: Verify
- Read both final `FreeRtosPlatformConfig.h` files, confirm `configUSE_MALLOC_FAILED_HOOK` is `1`
- Read both `osHooks.cpp` files, confirm `vApplicationMallocFailedHook` function exists and matches FreeRTOS expected signature (`extern "C" void vApplicationMallocFailedHook(void)`)

---

## T3 — Fix Linker Scripts: Stack Size + Overflow Guard (W3, ~7 min)

**Findings:** LD-01, LD-03

### Step 1: Read these files
1. `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld` — full file (145 lines)
2. `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld` — full file (145 lines)

Both files are identical except for the `LENGTH` values in the `MEMORY` block.

### Step 2: Make exactly 2 edits in EACH file (4 edits total)

**Edit 1 — LD-01: Increase stack size (line 14 in both files)**

Change:
```
_Min_Stack_Size = 0x400;    /* 1024 bytes */
```
To:
```
_Min_Stack_Size = 0x1000;   /* 4096 bytes — ISR nesting needs headroom */
```

**Edit 2 — LD-03: Add stack/heap overlap ASSERT (after line 137 in both files)**

After the line:
```
    } >RAM
```
(the closing of `._user_heap_stack` section at line 137), add:

```

    ASSERT((_estack - _Min_Stack_Size) >= . , "Error: heap and stack regions overlap")
```

This goes BEFORE the `/DISCARD/` section. The final result around line 137 should be:

```
    } >RAM

    ASSERT((_estack - _Min_Stack_Size) >= . , "Error: heap and stack regions overlap")

    /DISCARD/ :
```

### Step 3: Verify
- Read both final linker scripts
- Confirm `_Min_Stack_Size = 0x1000` on line 14 of both
- Confirm the `ASSERT` line exists between `._user_heap_stack` and `/DISCARD/` in both
- Confirm `_sstack = _estack - _Min_Stack_Size` still works correctly on line 11

---

## T4 — Fix clockConfig_f4 Flash Latency Readback (W1, ~5 min)

**Finding:** CLK-03

### Step 1: Read these files
1. `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` — full file (51 lines)
2. `platforms/stm32/bsp/bspClock/src/clockConfig_g4.cpp` — lines 65–74 only (reference pattern)

### Step 2: Make 1 edit in clockConfig_f4.cpp

After line 44:
```cpp
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
```

Add this readback verification line:
```cpp
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_3WS) {}
```

The result should look like:
```cpp
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_3WS) {}
```

**Note:** The F4 case is simpler than the G4 — it writes the full register (no read-modify-write), so a simple readback loop is correct. The G4 uses read-modify-write to preserve `DBG_SWEN`. DO NOT change the F4 to use read-modify-write.

### Step 3: Verify
- Read the final `clockConfig_f4.cpp`
- Confirm the readback loop is immediately after the `FLASH->ACR` write at (now) line 45
- Confirm the mask constant matches: `FLASH_ACR_LATENCY` and `FLASH_ACR_LATENCY_3WS`

---

## T5 — Cross-Verify All Fixes (W3, ~8 min)

**Purpose:** Read every patched file, confirm correctness, write report

### Checklist (read each file and check)

1. **FdCanTransceiver.cpp** (`platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp`)
   - [ ] `close()` has `fTxQueue.clear();` between `fDevice.stop();` and `setState(State::CLOSED);`
   - [ ] `mute()` has `fTxQueue.clear();` between `fMuted = true;` and `setState(State::MUTED);`
   - [ ] `canFrameSentAsyncCallback()`: the `sendAgain` guard is `if (State::OPEN == state)` — NO `State::INITIALIZED`

2. **FdCanDevice.h** (`platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h`)
   - [ ] `enterInitMode`, `leaveInitMode`, `configureBitTiming`, `configureMessageRam`, `configureGpio`, `enablePeripheralClock` are under `private:`
   - [ ] `fTxEventEnabled` is still accessible (public or in a public section)

3. **STM32F413ZHxx_FLASH.ld** (`platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld`)
   - [ ] `_Min_Stack_Size = 0x1000`
   - [ ] ASSERT for stack/heap overlap present

4. **STM32G474RExx_FLASH.ld** (`platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld`)
   - [ ] `_Min_Stack_Size = 0x1000`
   - [ ] ASSERT for stack/heap overlap present

5. **clockConfig_f4.cpp** (`platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp`)
   - [ ] Flash latency readback loop present after `FLASH->ACR` write
   - [ ] Constant is `FLASH_ACR_LATENCY_3WS` (not 4WS)

6. **FreeRtosPlatformConfig.h (F413ZH)** (`executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`)
   - [ ] `configUSE_MALLOC_FAILED_HOOK` is `1`

7. **FreeRtosPlatformConfig.h (G474RE)** (`executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`)
   - [ ] `configUSE_MALLOC_FAILED_HOOK` is `1`

8. **osHooks.cpp (F413ZH)** (`executables/referenceApp/platforms/nucleo_f413zh/main/src/osHooks/freertos/osHooks.cpp`)
   - [ ] `vApplicationMallocFailedHook` function exists

9. **osHooks.cpp (G474RE)** (`executables/referenceApp/platforms/nucleo_g474re/main/src/osHooks/freertos/osHooks.cpp`)
   - [ ] `vApplicationMallocFailedHook` function exists

### Output
Write verification results to `docs/fix-verification.md` with this format:

```markdown
# Fix Verification Report
**Date:** <date>
**Verifier:** W3 (automated)

| Check | File | Result | Notes |
|-------|------|--------|-------|
| CAN-01 | FdCanTransceiver.cpp | PASS/FAIL | ... |
| CAN-02 | FdCanTransceiver.cpp | PASS/FAIL | ... |
| CAN-03 | FdCanTransceiver.cpp | PASS/FAIL | ... |
| DEV-01 | FdCanDevice.h | PASS/FAIL | ... |
| LD-01a | STM32F413ZHxx_FLASH.ld | PASS/FAIL | ... |
| LD-01b | STM32G474RExx_FLASH.ld | PASS/FAIL | ... |
| LD-03a | STM32F413ZHxx_FLASH.ld | PASS/FAIL | ... |
| LD-03b | STM32G474RExx_FLASH.ld | PASS/FAIL | ... |
| CLK-03 | clockConfig_f4.cpp | PASS/FAIL | ... |
| F-01a | FreeRtosPlatformConfig.h (F413ZH) | PASS/FAIL | ... |
| F-01b | FreeRtosPlatformConfig.h (G474RE) | PASS/FAIL | ... |
```

---

## Execution Order

```
Phase 1 (parallel): T1 (W1), T2 (W2), T3 (W3)    — all independent, no file overlap
Phase 2 (parallel): T4 (W1)                        — independent of T1-T3
Phase 3 (serial):   T5 (W3)                        — depends on ALL of T1, T2, T3, T4
```

Timeline:
```
t=0      W1:T1(8min)   W2:T2(6min)   W3:T3(7min)
t=6min   —             W2:idle       —
t=7min   —             —             W3:wait
t=8min   W1:T4(5min)   —             W3:wait
t=13min  W1:idle       —             W3:T5(8min)
t=21min  Done
```

---

## Rules for All Workers

1. **Read before edit.** Always `Read` the target file before using `Edit`.
2. **No `git push`**, no `rm -rf`, no `flash`, no `st-flash`, no `STM32_Programmer_CLI`.
3. **Verify paths exist** before writing. Use `Glob` or `ls` first if uncertain.
4. **Match existing code style.** This project uses:
   - `// Copyright 2024 Contributors to the Eclipse Foundation` + `SPDX-License-Identifier: EPL-2.0`
   - Allman brace style with 4-space indentation
   - `#pragma once` for headers
5. **Do not modify test files** unless the test is explicitly broken by a production fix.
6. **Show before/after** in your output for each edit.
7. **One finding per edit** — do not combine unrelated changes in a single Edit call.

---

## Files Touched (complete list)

| Task | File | Action |
|------|------|--------|
| T1 | `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp` | 3 edits |
| T2 | `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h` | 1 edit |
| T2 | `executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` | 1 edit |
| T2 | `executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` | 1 edit |
| T3 | `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld` | 2 edits |
| T3 | `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld` | 2 edits |
| T4 | `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` | 1 edit |
| T5 | `docs/fix-verification.md` | create (new file) |

**Total: 8 source files edited, 1 report file created, 0 files deleted.**
