# STM32 Platform Port — Task Plan v3

**Branch:** `stm32-platform-port`
**Generated:** 2026-03-27
**Workers:** 3 Sonnet agents (W1, W2, W3), each task ≤ 10 minutes
**Blocked commands:** `git push`, `git push --force`, `rm -rf`, `docker rm`, `flash`, `st-flash`, `STM32_Programmer_CLI`

---

## Repository State

- **Commit:** `25e8dee6` — "Add STM32 platform support (NUCLEO-F413ZH + NUCLEO-G474RE)"
- **Changed files vs main:** 264 files, ~70,850 insertions
- **Working tree:** Clean (only `docs/` is untracked)
- **Build tools available:** cmake 4.2.3, gcc 15.2.0, arm-none-eabi-gcc 13.3.0, ninja 1.13.2, python 3.13.12, ctest 4.2.3
- **Missing:** clang-tidy (not needed for this plan)

## All 8 Bugs Verified PRESENT (2026-03-27)

| ID | Sev | File:Line | Issue |
|----|-----|-----------|-------|
| CAN-01 | MED | `FdCanTransceiver.cpp:93-94` | `close()` missing `fTxQueue.clear()` |
| CAN-02 | MED | `FdCanTransceiver.cpp:107-108` | `mute()` missing `fTxQueue.clear()` |
| CAN-03 | MED | `FdCanTransceiver.cpp:224` | `canFrameSentAsyncCallback()` allows `State::INITIALIZED` |
| DEV-01 | LOW | `FdCanDevice.h:186-194` | 6 internal methods public instead of private |
| LD-01 | MED | Both `.ld` line 14 | `_Min_Stack_Size = 0x400` too small for ISR nesting |
| LD-03 | LOW | Both `.ld` scripts | No ASSERT for heap/stack overlap |
| CLK-03 | LOW | `clockConfig_f4.cpp:44` | No flash latency readback (G4 already has it at line 74) |
| F-01 | LOW | Both `FreeRtosPlatformConfig.h:33` | `configUSE_MALLOC_FAILED_HOOK = 0` but hook exists |

---

## Phase 1 — Apply Fixes (3 workers in parallel, zero file overlap)

### T1: W1 — Fix FdCanTransceiver bugs (CAN-01, CAN-02, CAN-03)

**Estimated time:** 8 minutes
**Files touched:** 1 file only

**Step 1 — Read reference implementation:**
- Read `platforms/stm32/bsp/bxCanTransceiver/src/can/transceiver/bxcan/BxCanTransceiver.cpp` lines 90-116
  - Confirm `close()` at line 98 has `fTxQueue.clear();`
  - Confirm `mute()` at line 113 has `fTxQueue.clear();`

**Step 2 — Read target file:**
- Read `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp` full file (294 lines)

**Step 3 — Apply CAN-01:** Edit `FdCanTransceiver.cpp`
```
old_string:
    fDevice.stop();
    setState(State::CLOSED);

new_string:
    fDevice.stop();
    fTxQueue.clear();
    setState(State::CLOSED);
```

**Step 4 — Apply CAN-02:** Edit `FdCanTransceiver.cpp`
```
old_string:
    fMuted = true;
    setState(State::MUTED);

new_string:
    fMuted = true;
    fTxQueue.clear();
    setState(State::MUTED);
```

**Step 5 — Apply CAN-03:** Edit `FdCanTransceiver.cpp`
```
old_string:
                if ((State::OPEN == state) || (State::INITIALIZED == state))

new_string:
                if (State::OPEN == state)
```

**Step 6 — Verify:**
- Read `FdCanTransceiver.cpp` again
- Confirm `close()` (around line 93-95): `fDevice.stop();` then `fTxQueue.clear();` then `setState(State::CLOSED);`
- Confirm `mute()` (around line 107-109): `fMuted = true;` then `fTxQueue.clear();` then `setState(State::MUTED);`
- Confirm `canFrameSentAsyncCallback()` (around line 224): condition is `if (State::OPEN == state)` with NO `INITIALIZED`
- Grep the file for `INITIALIZED` — it should appear only in `init()` and `open()`, NOT in `canFrameSentAsyncCallback()`
- Compare close/mute against BxCanTransceiver.cpp lines 90-116 — patterns must now match

**Output:** Print before/after diffs for all 3 edits.

---

### T2: W2 — Fix FdCanDevice encapsulation + FreeRTOS malloc hook (DEV-01, F-01)

**Estimated time:** 7 minutes
**Files touched:** 3 files

**Step 1 — Read target files:**
- Read `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h` lines 175-198
- Read `platforms/stm32/bsp/bspCan/include/can/BxCanDevice.h` lines 200-220 (reference for private layout)
- Read `executables/referenceApp/platforms/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` line 33
- Read `executables/referenceApp/platforms/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` line 33

**Step 2 — Confirm hook functions exist:**
- Read `executables/referenceApp/platforms/nucleo_f413zh/main/src/osHooks/freertos/osHooks.cpp` — grep for `vApplicationMallocFailedHook`
- Read `executables/referenceApp/platforms/nucleo_g474re/main/src/osHooks/freertos/osHooks.cpp` — grep for `vApplicationMallocFailedHook`

**Step 3 — Apply DEV-01:** Edit `FdCanDevice.h`
Keep `fTxEventEnabled` public (it's accessed from FdCanTransceiver.cpp lines 163, 166, 170, 241, 246). Add `private:` before the internal methods:
```
old_string:
public:
    bool fTxEventEnabled{false}; ///< When true, TX buffer sets EFC=1 for TX Event FIFO

    void enterInitMode();
    void leaveInitMode();
    void configureBitTiming();
    void configureMessageRam();
    void configureGpio();
    void enablePeripheralClock();
};

new_string:
public:
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

**Step 4 — Apply F-01a:** Edit F413ZH `FreeRtosPlatformConfig.h`
```
old_string: #define configUSE_MALLOC_FAILED_HOOK            0
new_string: #define configUSE_MALLOC_FAILED_HOOK            1
```

**Step 5 — Apply F-01b:** Edit G474RE `FreeRtosPlatformConfig.h`
```
old_string: #define configUSE_MALLOC_FAILED_HOOK            0
new_string: #define configUSE_MALLOC_FAILED_HOOK            1
```

**Step 6 — Check mock compatibility:**
- Read `platforms/stm32/bsp/fdCanTransceiver/test/mock/include/can/FdCanDevice.h` — check if mock declares `enterInitMode` etc. The mock is test-only; if it has these as public, no change needed.

**Step 7 — Verify:**
- Read `FdCanDevice.h` — `enterInitMode` through `enablePeripheralClock` must be under `private:`
- Read `FdCanDevice.h` — `fTxEventEnabled` must still be under `public:`
- Read both `FreeRtosPlatformConfig.h` — `configUSE_MALLOC_FAILED_HOOK` must be `1`

**Output:** Print before/after diffs for all 3 edits.

---

### T3: W3 — Fix linker scripts + clockConfig flash readback (LD-01, LD-03, CLK-03)

**Estimated time:** 8 minutes
**Files touched:** 3 files

**Step 1 — Read target files:**
- Read `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld` full file (145 lines)
- Read `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld` full file (145 lines)
- Read `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` full file (51 lines)
- Read `platforms/stm32/bsp/bspClock/src/clockConfig_g4.cpp` lines 65-74 (reference for readback pattern)

**Step 2 — Apply LD-01a:** Edit `STM32F413ZHxx_FLASH.ld`
```
old_string: _Min_Stack_Size = 0x400;    /* 1024 bytes */
new_string: _Min_Stack_Size = 0x1000;   /* 4096 bytes — ISR nesting needs headroom */
```

**Step 3 — Apply LD-01b:** Edit `STM32G474RExx_FLASH.ld`
```
old_string: _Min_Stack_Size = 0x400;    /* 1024 bytes */
new_string: _Min_Stack_Size = 0x1000;   /* 4096 bytes — ISR nesting needs headroom */
```

**Step 4 — Apply LD-03a:** Edit `STM32F413ZHxx_FLASH.ld` — insert ASSERT after `._user_heap_stack` block, before `/DISCARD/`
```
old_string:
    } >RAM

    /DISCARD/ :

new_string:
    } >RAM

    ASSERT((_estack - _Min_Stack_Size) >= . , "Error: heap and stack regions overlap")

    /DISCARD/ :
```
This matches the `._user_heap_stack` closing `} >RAM` at line 137, not the `.bss` closing at line 127. Use sufficient context to be unique — include lines before and after.

**Step 5 — Apply LD-03b:** Same ASSERT in `STM32G474RExx_FLASH.ld` at the same location. Use the same old_string/new_string as Step 4.

**Step 6 — Apply CLK-03:** Edit `clockConfig_f4.cpp`
```
old_string:
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    // AHB = SYSCLK, APB1 = SYSCLK/2, APB2 = SYSCLK

new_string:
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    // Verify flash latency readback before switching clock (RM0090 section 3.5.1)
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_3WS) {}

    // AHB = SYSCLK, APB1 = SYSCLK/2, APB2 = SYSCLK
```
(This mirrors the G4 readback pattern at `clockConfig_g4.cpp:73-74`.)

**Step 7 — Verify:**
- Read both linker scripts — confirm `_Min_Stack_Size = 0x1000` at line 14 and ASSERT present between `._user_heap_stack` and `/DISCARD/`
- Read `clockConfig_f4.cpp` — confirm `while` readback loop on line after `FLASH->ACR` write
- Compare with `clockConfig_g4.cpp` line 73-74 — pattern should match

**Output:** Print before/after diffs for all 5 edits.

---

## Phase 2 — Build Verification (2 workers in parallel, after Phase 1 completes)

### T4: W1 — Build and run STM32 host unit tests

**Estimated time:** 10 minutes
**Files touched:** 0 source files (output only)

**Step 1 — Configure:**
```bash
cmake --preset tests-stm32-debug
```

**Step 2 — Build:**
```bash
cmake --build --preset tests-stm32-debug
```

**Step 3 — Run tests:**
```bash
ctest --preset tests-stm32-debug --output-on-failure
```

**Step 4 — Check results:**
- All tests pass (expect ~152 test cases)
- No build warnings from STM32 BSP files (grep build log for `warning:` in STM32 paths)
- Zero test failures

**Step 5 — If tests fail:**
- Capture full error output
- Check if DEV-01 (FdCanDevice encapsulation) broke something: look for errors in `FdCanTransceiverTest.cpp` or `FdCanTransceiverIntegrationTest.cpp`
- Check if the test mock at `platforms/stm32/bsp/fdCanTransceiver/test/mock/include/can/FdCanDevice.h` needs its methods to remain public
- If mock needs fixing: the 6 methods in the mock should stay public since mock is used for testing only

**Output:** Write results to `docs/test-results-stm32.txt` containing:
- cmake configure output (last 20 lines)
- build output (last 30 lines, or full if errors)
- ctest output (full)
- PASS/FAIL verdict

---

### T5: W2 — Cross-compile both boards with ARM GCC

**Estimated time:** 10 minutes
**Files touched:** 0 source files (output only)

**Step 1 — Configure + Build F413ZH:**
```bash
cmake --preset nucleo-f413zh-freertos-gcc
cmake --build --preset nucleo-f413zh-freertos-gcc
```

**Step 2 — Configure + Build G474RE:**
```bash
cmake --preset nucleo-g474re-freertos-gcc
cmake --build --preset nucleo-g474re-freertos-gcc
```

**Step 3 — Check binary sizes:**
```bash
arm-none-eabi-size build/nucleo-f413zh-freertos-gcc/RelWithDebInfo/*.elf
arm-none-eabi-size build/nucleo-g474re-freertos-gcc/RelWithDebInfo/*.elf
```
Limits:
- F413ZH: Flash < 1536 KB (0x180000), RAM < 320 KB (0x50000)
- G474RE: Flash < 512 KB (0x80000), RAM < 128 KB (0x20000)

**Step 4 — If build fails:**
- Capture full error output
- Check if DEV-01 (FdCanDevice private methods) caused `FdCanTransceiver.cpp` to fail accessing now-private methods — if so, those methods are called via `fDevice.` in FdCanTransceiver.cpp so check lines 40-60 for `fDevice.enterInitMode()` etc.
  - If they are: FdCanTransceiver is NOT a friend and cannot call private methods → the DEV-01 fix needs `friend class FdCanTransceiver;` in FdCanDevice.h, OR these methods need a different access pattern
- Report findings in output file

**Output:** Write results to `docs/build-results-arm.txt` containing:
- Build output (last 30 lines per target, or full if errors)
- `arm-none-eabi-size` output for each binary
- PASS/FAIL verdict per board

---

## Phase 3 — Final Verification (1 worker, after Phase 2 completes)

### T6: W3 — Cross-verify all fixes and write final report

**Estimated time:** 8 minutes
**Files touched:** 0 source files (output only)

**Read each patched file and verify:**

| # | File | Check |
|---|------|-------|
| 1 | `platforms/stm32/bsp/fdCanTransceiver/src/can/transceiver/fdcan/FdCanTransceiver.cpp` | `close()` has `fTxQueue.clear();` between `fDevice.stop();` and `setState(State::CLOSED);` |
| 2 | Same file | `mute()` has `fTxQueue.clear();` between `fMuted = true;` and `setState(State::MUTED);` |
| 3 | Same file | `canFrameSentAsyncCallback()`: condition is `if (State::OPEN == state)` — NO `INITIALIZED` |
| 4 | `platforms/stm32/bsp/bspCan/include/can/FdCanDevice.h` | `enterInitMode` through `enablePeripheralClock` under `private:` |
| 5 | Same file | `fTxEventEnabled` still `public` |
| 6 | `platforms/stm32/bsp/bspMcu/linker/STM32F413ZHxx_FLASH.ld` | `_Min_Stack_Size = 0x1000` |
| 7 | Same file | ASSERT for stack/heap overlap present |
| 8 | `platforms/stm32/bsp/bspMcu/linker/STM32G474RExx_FLASH.ld` | `_Min_Stack_Size = 0x1000` |
| 9 | Same file | ASSERT for stack/heap overlap present |
| 10 | `platforms/stm32/bsp/bspClock/src/clockConfig_f4.cpp` | Readback loop after `FLASH->ACR` write |
| 11 | `executables/referenceApp/platforms/nucleo_f413zh/.../FreeRtosPlatformConfig.h` | `configUSE_MALLOC_FAILED_HOOK` is `1` |
| 12 | `executables/referenceApp/platforms/nucleo_g474re/.../FreeRtosPlatformConfig.h` | `configUSE_MALLOC_FAILED_HOOK` is `1` |
| 13 | `docs/test-results-stm32.txt` | All tests passed |
| 14 | `docs/build-results-arm.txt` | Both ARM builds succeeded, binary sizes within limits |

**Output:** Write `docs/fix-verification.md` with this markdown table:

```markdown
# Fix Verification Report
**Date:** 2026-03-27
**Branch:** stm32-platform-port
**Base commit:** 25e8dee6

| # | Check | File | Result | Notes |
|---|-------|------|--------|-------|
| 1 | CAN-01 close() fTxQueue.clear() | FdCanTransceiver.cpp | PASS/FAIL | ... |
| 2 | CAN-02 mute() fTxQueue.clear() | FdCanTransceiver.cpp | PASS/FAIL | ... |
| 3 | CAN-03 no INITIALIZED guard | FdCanTransceiver.cpp | PASS/FAIL | ... |
| 4 | DEV-01 methods private | FdCanDevice.h | PASS/FAIL | ... |
| 5 | DEV-01 fTxEventEnabled public | FdCanDevice.h | PASS/FAIL | ... |
| 6 | LD-01a stack size F413 | STM32F413ZHxx_FLASH.ld | PASS/FAIL | ... |
| 7 | LD-03a ASSERT F413 | STM32F413ZHxx_FLASH.ld | PASS/FAIL | ... |
| 8 | LD-01b stack size G474 | STM32G474RExx_FLASH.ld | PASS/FAIL | ... |
| 9 | LD-03b ASSERT G474 | STM32G474RExx_FLASH.ld | PASS/FAIL | ... |
| 10 | CLK-03 readback loop | clockConfig_f4.cpp | PASS/FAIL | ... |
| 11 | F-01a malloc hook F413 | FreeRtosPlatformConfig.h | PASS/FAIL | ... |
| 12 | F-01b malloc hook G474 | FreeRtosPlatformConfig.h | PASS/FAIL | ... |
| 13 | STM32 unit tests | test-results-stm32.txt | PASS/FAIL | ... |
| 14 | ARM cross-compile | build-results-arm.txt | PASS/FAIL | ... |
```

---

## Execution Schedule

```
Phase 1 (parallel):  W1:T1(8min)   W2:T2(7min)   W3:T3(8min)
Phase 2 (parallel):  W1:T4(10min)  W2:T5(10min)  W3:idle
Phase 3 (serial):    W3:T6(8min)

t=0      W1→T1         W2→T2         W3→T3
t=8min   W1→T4         W2→T5         W3 waits for T4+T5
t=18min  W1 done       W2 done       W3→T6
t=26min  All complete
```

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

**Total: 7 source files edited, 3 report files created, 0 files deleted.**

## Rules for All Workers

1. **Read before edit.** Always `Read` the target file before using `Edit`.
2. **Blocked commands:** No `git push`, no `rm -rf`, no `flash`, no `st-flash`, no `STM32_Programmer_CLI`.
3. **Verify paths exist** before writing. Use `Glob` or `ls` first if uncertain.
4. **Match existing code style:** Allman braces, 4-space indentation.
5. **Do not modify test files** unless the test is explicitly broken by a production fix.
6. **One finding per edit** — do not combine unrelated changes in a single Edit call.
7. **Print SUMMARY at end** of each task with what was done and outcome.

## Risk Assessment: DEV-01 is Safe

**Verified:** `FdCanTransceiver.cpp` does NOT call any of the 6 methods being made private. They are only called internally within `FdCanDevice.cpp` (lines 179-188, 207, 218). The DEV-01 fix is safe — no `friend class` needed, no external callers exist.
