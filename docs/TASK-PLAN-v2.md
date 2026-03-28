# STM32 Platform Port — Task Plan v2

**Branch:** `stm32-platform-port`
**Generated:** 2026-03-27
**Workers:** 3 Sonnet agents (W1, W2, W3), each task ≤ 10 minutes
**Blocked commands:** `git push`, `rm -rf`, `flash`, `st-flash`, `STM32_Programmer_CLI`

---

## Repository State

- **Commit:** `25e8dee6` — "Add STM32 platform support (NUCLEO-F413ZH + NUCLEO-G474RE)"
- **Changed files vs main:** 264 files, ~70,850 insertions
- **Working tree:** Clean (only `docs/` is untracked)
- **Build tools available:** cmake 4.2.3, gcc 15.2.0, arm-none-eabi-gcc 13.3.0, ninja 1.13.2

## Prior Work (already completed)

Three review documents exist in `docs/`:
- `stm32-bsp-findings.md` — BSP comparison vs S32K reference, 8 findings
- `stm32-app-findings.md` — Application layer review, 7 findings
- `stm32-port-verification.md` — 15-area verification plan, 590 lines

## Outstanding Bugs (all confirmed still present)

| ID | Severity | File | Issue |
|----|----------|------|-------|
| CAN-01 | MEDIUM | `FdCanTransceiver.cpp:93-94` | `close()` missing `fTxQueue.clear()` |
| CAN-02 | MEDIUM | `FdCanTransceiver.cpp:107-108` | `mute()` missing `fTxQueue.clear()` |
| CAN-03 | MEDIUM | `FdCanTransceiver.cpp:224` | `canFrameSentAsyncCallback()` has invalid `State::INITIALIZED` guard |
| DEV-01 | LOW | `FdCanDevice.h:186-194` | 6 internal methods declared `public:` instead of `private:` |
| LD-01 | MEDIUM | Both `.ld` linker scripts line 14 | `_Min_Stack_Size = 0x400` (1 KB too small for ISR nesting) |
| LD-03 | LOW | Both `.ld` linker scripts | No ASSERT for heap/stack overlap |
| CLK-03 | LOW | `clockConfig_f4.cpp:44` | No flash latency readback verification loop |
| F-01 | LOW | Both `FreeRtosPlatformConfig.h` line 33 | `configUSE_MALLOC_FAILED_HOOK = 0` but hook function exists |

---

## Task Breakdown

### Phase 1 — Apply Fixes (3 workers in parallel, no file overlap)

#### T1: W1 — Fix FdCanTransceiver bugs (CAN-01, CAN-02, CAN-03) [~8 min]

**Read first:**
1. `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp` — full file (294 lines)
2. `platforms/stm32/bsp/bxCanTransceiver/src/can/transceiver/bxcan/BxCanTransceiver.cpp` — lines 86-116 and 270-285 (reference)

**Edit 1 (CAN-01):** In `FdCanTransceiver.cpp`, between lines 93 and 94:
```
old: fDevice.stop();
     setState(State::CLOSED);
new: fDevice.stop();
     fTxQueue.clear();
     setState(State::CLOSED);
```

**Edit 2 (CAN-02):** In `FdCanTransceiver.cpp`, between lines 107 and 108:
```
old: fMuted = true;
     setState(State::MUTED);
new: fMuted = true;
     fTxQueue.clear();
     setState(State::MUTED);
```

**Edit 3 (CAN-03):** In `FdCanTransceiver.cpp`, line 224:
```
old: if ((State::OPEN == state) || (State::INITIALIZED == state))
new: if (State::OPEN == state)
```

**Verify after edits:**
- Read final file, confirm all 3 changes
- Grep for `INITIALIZED` — must only appear in `init()` and `open()`, NOT in `canFrameSentAsyncCallback()`
- Compare `close()`/`mute()` with `BxCanTransceiver.cpp` lines 90-116

---

#### T2: W2 — Fix FdCanDevice encapsulation + FreeRTOS malloc hook (DEV-01, F-01) [~7 min]

**Read first:**
1. `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h` — full file (focus lines 179-195)
2. `platforms/stm32/bsp/bspCan/include/can/BxCanDevice.h` — lines 206-218 (reference)
3. `executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` — line 33
4. `executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` — line 33
5. `executables/referenceApp/platforms/nucleo_f413zh/main/src/osHooks/freertos/osHooks.cpp` — confirm hook function exists
6. `executables/referenceApp/platforms/nucleo_g474re/main/src/osHooks/freertos/osHooks.cpp` — confirm hook function exists

**Edit 1 (DEV-01):** In `FdCanDevice.h`, change lines 186-195:
```
old: public:
         bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

         void enterInitMode();
         void leaveInitMode();
         void configureBitTiming();
         void configureMessageRam();
         void configureGpio();
         void enablePeripheralClock();
     };
new:     bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

     private:
         void enterInitMode();
         void leaveInitMode();
         void configureBitTiming();
         void configureMessageRam();
         void configureGpio();
         void enablePeripheralClock();
     };
```
NOTE: `fTxEventEnabled` must stay accessible. The line before `public:` at 186 is the `private:` block starting at 179. Remove the `public:` on line 186 so `fTxEventEnabled` inherits `private`, then add a targeted access — OR keep `fTxEventEnabled` under the existing `public:` and add a new `private:` before `enterInitMode()`. Choose the approach that keeps `fTxEventEnabled` public (it's accessed from `FdCanTransceiver.cpp` lines 163, 166, 170, 241, 246).

Best approach: Keep the `public:` for `fTxEventEnabled`, insert `private:` before `enterInitMode()`:
```
public:
    bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

private:
    void enterInitMode();
    ...
```

**Edit 2 (F-01a):** In F413ZH `FreeRtosPlatformConfig.h`, line 33:
```
old: #define configUSE_MALLOC_FAILED_HOOK            0
new: #define configUSE_MALLOC_FAILED_HOOK            1
```

**Edit 3 (F-01b):** In G474RE `FreeRtosPlatformConfig.h`, line 33:
```
old: #define configUSE_MALLOC_FAILED_HOOK            0
new: #define configUSE_MALLOC_FAILED_HOOK            1
```

**Verify after edits:**
- Read `FdCanDevice.h` — `enterInitMode` through `enablePeripheralClock` must be under `private:`
- Read `FdCanDevice.h` — `fTxEventEnabled` must still be under `public:`
- Read both `FreeRtosPlatformConfig.h` — `configUSE_MALLOC_FAILED_HOOK` must be `1`
- Read both `osHooks.cpp` — confirm `vApplicationMallocFailedHook` function exists with correct signature
- Read `platforms/stm32/bsp/fdCanTransceiver/test/mock/include/can/FdCanDevice.h` — check if mock needs updating (if mock has `enterInitMode` as public, no change needed since mock is test-only)

---

#### T3: W3 — Fix linker scripts + clockConfig flash readback (LD-01, LD-03, CLK-03) [~8 min]

**Read first:**
1. `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld` — full file (145 lines)
2. `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld` — full file (145 lines)
3. `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` — full file (51 lines)
4. `platforms/stm32/bsp/bspClock/src/clockConfig_g4.cpp` — lines 65-74 only (reference)

**Edit 1 (LD-01a):** In `STM32F413ZHxx_FLASH.ld`, line 14:
```
old: _Min_Stack_Size = 0x400;    /* 1024 bytes */
new: _Min_Stack_Size = 0x1000;   /* 4096 bytes — ISR nesting needs headroom */
```

**Edit 2 (LD-01b):** In `STM32G474RExx_FLASH.ld`, line 14:
```
old: _Min_Stack_Size = 0x400;    /* 1024 bytes */
new: _Min_Stack_Size = 0x1000;   /* 4096 bytes — ISR nesting needs headroom */
```

**Edit 3 (LD-03a):** In `STM32F413ZHxx_FLASH.ld`, after line 137 (`} >RAM` closing `._user_heap_stack`), add:
```

    ASSERT((_estack - _Min_Stack_Size) >= . , "Error: heap and stack regions overlap")
```
This goes BEFORE the `/DISCARD/` section.

**Edit 4 (LD-03b):** Same ASSERT in `STM32G474RExx_FLASH.ld` at the same location.

**Edit 5 (CLK-03):** In `clockConfig_f4.cpp`, after line 44:
```
old: FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
new: FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
     while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_3WS) {}
```

**Verify after edits:**
- Read both linker scripts — confirm `_Min_Stack_Size = 0x1000` and ASSERT present
- Read `clockConfig_f4.cpp` — confirm readback loop present after FLASH->ACR write
- Confirm `_sstack = _estack - _Min_Stack_Size` still appears correctly
- Compare `clockConfig_f4.cpp` readback pattern against `clockConfig_g4.cpp` line ~70

---

### Phase 2 — Build Verification (2 workers in parallel) [after Phase 1 completes]

#### T4: W1 — Build and run STM32 unit tests [~10 min]

**Commands to execute (sequentially):**
1. `cmake --preset tests-stm32-debug` — configure
2. `cmake --build --preset tests-stm32-debug` — build
3. `ctest --preset tests-stm32-debug --output-on-failure` — run tests

**Check:**
- All tests pass (expect ~152 test cases)
- No build warnings in STM32 BSP files
- Zero test failures

**Output:** Write results to `docs/test-results-stm32.txt`

**If tests fail:**
- Capture full error output
- Identify which test failed and what assertion
- Check if failure is related to Phase 1 edits (especially DEV-01 mock compatibility)
- Write failure analysis to `docs/test-results-stm32.txt`

---

#### T5: W2 — Cross-compile both boards with ARM GCC [~10 min]

**Commands to execute (sequentially):**
1. `cmake --preset nucleo-f413zh-freertos-gcc` — configure F413ZH
2. `cmake --build --preset nucleo-f413zh-freertos-gcc` — build F413ZH
3. `cmake --preset nucleo-g474re-freertos-gcc` — configure G474RE
4. `cmake --build --preset nucleo-g474re-freertos-gcc` — build G474RE

**Check:**
- Both builds succeed with 0 errors
- Binary files produced (`.elf`)
- Run `arm-none-eabi-size <elf>` on both binaries — confirm Flash < memory limit, RAM < memory limit
  - F413ZH: Flash < 1536 KB, RAM < 320 KB
  - G474RE: Flash < 512 KB, RAM < 128 KB

**Output:** Write results to `docs/build-results-arm.txt`

**If build fails:**
- Capture full error output
- Check if DEV-01 (FdCanDevice encapsulation change) caused compilation failure in FdCanTransceiver.cpp accessing private methods
- Write failure analysis to output file

---

### Phase 3 — Cross-Verify All Fixes (1 worker, depends on Phase 1+2)

#### T6: W3 — Verify all fixes and write final report [~8 min]

**Read each patched file and check:**

1. `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp`
   - [ ] `close()` has `fTxQueue.clear();` between `fDevice.stop();` and `setState(State::CLOSED);`
   - [ ] `mute()` has `fTxQueue.clear();` between `fMuted = true;` and `setState(State::MUTED);`
   - [ ] `canFrameSentAsyncCallback()`: guard is `if (State::OPEN == state)` — NO `State::INITIALIZED`

2. `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h`
   - [ ] `enterInitMode`, `leaveInitMode`, `configureBitTiming`, `configureMessageRam`, `configureGpio`, `enablePeripheralClock` are under `private:`
   - [ ] `fTxEventEnabled` is still `public`

3. `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld`
   - [ ] `_Min_Stack_Size = 0x1000`
   - [ ] ASSERT for stack/heap overlap present

4. `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld`
   - [ ] `_Min_Stack_Size = 0x1000`
   - [ ] ASSERT for stack/heap overlap present

5. `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp`
   - [ ] Flash latency readback loop present after `FLASH->ACR` write

6. `executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`
   - [ ] `configUSE_MALLOC_FAILED_HOOK` is `1`

7. `executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`
   - [ ] `configUSE_MALLOC_FAILED_HOOK` is `1`

8. Read `docs/test-results-stm32.txt` — confirm all tests passed
9. Read `docs/build-results-arm.txt` — confirm both ARM builds succeeded

**Output:** Write `docs/fix-verification.md` with this table:

```markdown
# Fix Verification Report
**Date:** 2026-03-27
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
| BUILD-STM32 | Unit tests | PASS/FAIL | ... |
| BUILD-F413ZH | ARM cross-compile | PASS/FAIL | ... |
| BUILD-G474RE | ARM cross-compile | PASS/FAIL | ... |
```

---

## Execution Schedule

```
Phase 1 (parallel):  W1:T1(8min)   W2:T2(7min)   W3:T3(8min)
Phase 2 (parallel):  W1:T4(10min)  W2:T5(10min)  W3:idle
Phase 3 (serial):    W3:T6(8min)
```

```
t=0      W1→T1         W2→T2         W3→T3
t=8min   W1→T4         W2→T5         W3 waits for T4+T5
t=18min  W1 done       W2 done       W3→T6
t=26min  All done
```

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
| T3 | `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` | 1 edit |
| T4 | `docs/test-results-stm32.txt` | create (output) |
| T5 | `docs/build-results-arm.txt` | create (output) |
| T6 | `docs/fix-verification.md` | create (output) |

**Total: 8 source files edited, 3 report files created, 0 files deleted.**

---

## Rules for All Workers

1. **Read before edit.** Always `Read` the target file before using `Edit`.
2. **No `git push`**, no `rm -rf`, no `flash`, no `st-flash`, no `STM32_Programmer_CLI`.
3. **Verify paths exist** before writing. Use `Glob` or `ls` first if uncertain.
4. **Match existing code style:** Allman braces, 4-space indentation, `#pragma once` for headers.
5. **Do not modify test files** unless the test is explicitly broken by a production fix.
6. **One finding per edit** — do not combine unrelated changes in a single Edit call.
7. **Show before/after** in output for each edit made.
