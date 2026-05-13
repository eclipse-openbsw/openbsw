# STM32 Platform Port — PR Strategy

**Project:** eclipse-openbsw/openbsw
**Author:** nhuvaoanh123 (An Dao)
**Date:** 2026-04-16
**Status:** DRAFT — internal review before upstream proposal

---

## Background

The original monolith PR (#408) was closed and split into 10 stacked PRs
(#413-#422). The split was done quickly without a written strategy, leading to
three problems flagged by the upstream maintainer (rolandreichweinbmw):

1. **CMSIS duplication** — identical CMSIS 6.1.0 headers copied per-platform
   instead of shared via `libs/3rdparty/`.
2. **Broken presets** — PR #1 ships cmake presets (`nucleo-f413zh-*`,
   `nucleo-g474re-freertos-*`) that reference files not introduced until
   PRs #6/#8. The reviewer tried them, they fail.
3. **Unwired tests** — many test source files exist but are not registered in
   CMakeLists, so CI cannot run them.

This document defines a restructured series (PR 0 + PRs 1-9 = 10 PRs total)
where every PR has a clear scope, is independently testable, and introduces no
broken references.

---

## Principles

1. **Each PR must pass CI on its own.** No preset, target, or include may
   reference a file that does not exist in that PR's cumulative tree.
2. **Presets appear with their scaffolding.** A `nucleo-*-freertos-gcc` preset
   is introduced in the same PR that provides `Options.cmake`,
   `FreeRtosPlatformConfig.h`, `main.cpp`, and the linker script.
3. **Tests are wired when introduced.** If a PR adds test source files, it also
   registers them in the unit test build so CI runs them.
4. **Shared code goes to shared locations.** CMSIS headers used by multiple
   platforms live in `libs/3rdparty/cmsis/`, not duplicated per-platform.
5. **No behavior-changing shared code modifications.** Platform-specific
   configuration (DoCAN addressing, UDS sessions) is handled via board-level
   config headers. Shared source files may receive minimal additive changes
   (e.g. reading from a config header instead of hardcoding values), but
   existing platform behavior is never altered.
6. **Clean branch history.** Each PR branch contains only its own commits. No
   unrelated main-branch work (middleware, clang-tidy) mixed in.

---

## PR Series

### PR 0 — CMSIS Shared Library (Preparatory Refactor)

**Goal:** Extract the duplicated CMSIS headers into a shared location so both
s32k1xx and STM32 can use them. Pure refactor — zero new functionality, zero
behavior change.

#### Background

Both s32k1xx (NXP S32K148) and the STM32 chips are Cortex-M4F. CMSIS-Core
headers (`core_cm4.h`, `cmsis_gcc.h`, etc.) are ARM-defined, not
vendor-specific — they describe the Cortex-M4 core itself. Both platform
copies are CMSIS 6.1.0: 6 of 9 files are byte-identical, 3 files differ only
in Doxygen comment style (`@file` vs `\file`). No functional difference.

`libs/3rdparty/` already hosts shared dependencies (etl, freeRtos, googletest,
lwip, printf, threadx). Adding `libs/3rdparty/cmsis/` follows the existing
pattern.

#### Scope

| Area | What |
|------|------|
| Move CMSIS | `platforms/s32k1xx/bsp/bspMcu/include/3rdparty/cmsis/` -> `libs/3rdparty/cmsis/` (9 files) |
| Normalize | Fix trivial Doxygen style diffs (`@file` vs `\file`) across 3 files: `cmsis_clang.h`, `cmsis_gcc.h`, `core_cm4.h` |
| Delete STM32 copy | Remove `platforms/stm32/bsp/bspMcu/include/3rdparty/cmsis/` (duplicate) |
| NOTICE.md | Two CMSIS entries (lines 48-49) -> single entry pointing to `libs/3rdparty/cmsis/LICENSE` |

#### Impact analysis (6 files touched, all in s32k1xx or build config)

| File | Change | Why |
|------|--------|-----|
| `platforms/s32k1xx/bsp/bspMcu/CMakeLists.txt` | Add `${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis` and `${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis/m-profile` to `target_include_directories` | CMSIS headers are no longer under this module's `include/` tree |
| `platforms/s32k1xx/bsp/bspMcu/include/mcu/mcu.h:10` | `#include "3rdparty/cmsis/core_cm4.h"` -> `#include "core_cm4.h"` | Hardcoded relative path no longer resolves; bare include resolves via new include path |
| `platforms/s32k1xx/bsp/bspMcu/include/mcu/mcu.h:14` | Remove dead `#include "mcu/core_cm4.h"` code path | No such file exists anywhere in the repo; dead branch |
| `platforms/s32k1xx/bsp/bspMcu/module.spec` | Update `format_check_exclude` and `sca_exclude` globs | CMSIS files are no longer under `include/3rdparty/cmsis/` |
| `platforms/stm32/bsp/bspMcu/CMakeLists.txt` | Change include path from `include/3rdparty/cmsis` to `${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis` | Point to shared location |
| `NOTICE.md` | Consolidate two CMSIS entries to one | Single shared copy, single license path |

#### What does NOT change

- STM32 device headers (`stm32g474xx.h`, `stm32f413xx.h`) use bare
  `#include "core_cm4.h"` — resolved via include path, no code change
- All CMSIS internal includes are relative within the directory — moving
  the directory as a unit preserves them
- Nothing outside `platforms/` uses CMSIS-Core (`libs/3rdparty/etl/` has
  `cmsis_os2.h` which is CMSIS-RTOS2, unrelated)
- The s32k1xx `mcu.h` MPU workaround (`#undef __MPU_PRESENT`) is in
  platform code, not in the CMSIS files — unaffected

#### Explicitly excluded

- No new platform code
- No STM32-specific files (beyond deleting the duplicate copy)
- No new cmake targets
- No behavioral changes to any platform

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| s32k1xx tests pass | `ctest --preset tests-s32k1xx-debug` | All existing tests pass (zero regressions) |
| s32k1xx FreeRTOS build | `cmake --preset s32k148-freertos-gcc && build` | Cross-compile succeeds |
| s32k1xx ThreadX build | `cmake --preset s32k148-threadx-gcc && build` | Cross-compile succeeds |
| Formatting | `treefmt` | Clean |

If any include path is wrong, the build fails immediately at configure or
compile time — not a subtle bug.

#### Why separate

- Pure refactor is easy to review (small diff, clear intent)
- If it breaks s32k1xx, easy to bisect (only change in the PR)
- Once merged, the CMSIS concern never comes up in STM32 reviews
- Unblocks PR 1 from the reviewer's duplication objection

---

### PR 1 — STM32 MCU Foundation

**Goal:** Establish the MCU header layer so the repo can compile
STM32-specific unit tests. Uses shared CMSIS from PR 0.

#### Scope

| Area | What |
|------|------|
| STM32 device headers | `stm32f413xx.h`, `stm32g474xx.h`, `system_stm32f4xx.h`, `system_stm32g4xx.h` under `platforms/stm32/bsp/bspMcu/include/` |
| Startup assembly | `startup_stm32f413xx.s`, `startup_stm32g474xx.s` (FPU enabled, weak `SystemInit`) |
| Chip cmake | `platforms/stm32/cmake/stm32f413zh.cmake`, `stm32g474re.cmake` (set `STM32_FAMILY`, `CAN_TYPE`, startup source) |
| Platform top-level | `platforms/stm32/CMakeLists.txt` — chip selection, `add_subdirectory(bsp)` |
| bspMcu | `platforms/stm32/bsp/bspMcu/CMakeLists.txt` — static lib, links PUBLIC `platform`, includes CMSIS from `libs/3rdparty/cmsis/` |
| mcu.h / typedefs.h | Platform entry-point header, `softwareSystemReset.cpp` |
| Unit test preset | `tests-stm32-debug` configure + build + test presets |
| Root CMakeLists | `NUCLEO_F413ZH` / `NUCLEO_G474RE` platform routing, stm32 unit test section (empty initially) |
| NOTICE.md | Add STM32 device header entries (ST license) |

#### Explicitly excluded

- No `nucleo-*-freertos-gcc` or `nucleo-*-threadx-gcc` application presets
- No `executables/referenceApp/platforms/nucleo_g474re/` or `nucleo_f413zh/`
- No unrelated commits (middleware, clang-tidy, ethernet, logger)

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| STM32 test preset configures | `cmake --preset tests-stm32-debug` | Success (0 errors) |
| STM32 test preset builds | `cmake --build --preset tests-stm32-debug` | Success (0 tests, 0 errors) |
| s32k1xx tests unbroken | `ctest --preset tests-s32k1xx-debug` | All existing tests pass |
| Formatting | `treefmt` | Clean |

---

### PR 2 — BSP Peripheral Drivers + Unit Tests

**Goal:** Add the basic peripheral driver layer with full unit test coverage.

#### Scope

| Area | What |
|------|------|
| bspClock | Clock config for F4 + G4 families, `bspClock` static lib |
| bspIo | GPIO driver, `bspIo` static lib |
| bspUart | UART driver, `bspUart` static lib / INTERFACE (UT mode) |
| bspTimer | System timer, `bspTimer` static lib |
| bspAdc | ADC driver, `bspAdc` static lib |
| bspEepromDriver | Flash EEPROM driver, `bspEepromDriver` static lib |
| bspInterruptsImpl | Interrupt manager, `bspInterruptsImpl` static lib |
| etlImpl | ETL platform bridge (print + clocks), `etlImpl` static lib |
| socBsp | INTERFACE aggregate target linking all core BSP libs |
| Unit tests (wired) | `bspIoTest` (GpioTest, GpioStressTest), `bspUartTest` (UartTest), `bspAdcTest` (AdcTest, AdcExtendedTest), `bspEepromDriverTest` (FlashEepromDriverTest, FlashEepromStressTest), `bspClockTest` (ClockConfigTest) |

#### Dependencies

- PR 0: shared CMSIS in `libs/3rdparty/cmsis/`
- PR 1: `bspMcu`, chip cmake, `tests-stm32-debug` preset

#### CMake registration (required in this PR)

- New per-suite `test/CMakeLists.txt` for: `bspIo`, `bspUart`, `bspAdc`,
  `bspEepromDriver`, `bspClock`
- `bspUart/test/CMakeLists.txt` updated to build `UartTest.cpp` (not only
  `IncludeTest.cpp`)
- Root `CMakeLists.txt` adds `add_subdirectory()` for each test directory
  under the `stm32` unit test section

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| Unit tests run | `ctest --preset tests-stm32-debug` | All BSP tests pass |
| Test count | | >= 5 test executables discovered: `bspIoTest`, `bspUartTest`, `bspAdcTest`, `bspEepromDriverTest`, `bspClockTest` |
| Prior tests unbroken | `ctest --preset tests-s32k1xx-debug` | Pass |
| Formatting | `treefmt` | Clean |

---

### PR 3 — CAN Device Drivers + Unit Tests

**Goal:** Add the low-level CAN peripheral drivers (bxCAN for F4, FDCAN for G4).

#### Scope

| Area | What |
|------|------|
| BxCanDevice | `bsp/bspCan/` — bxCAN register-level driver (F4) |
| FdCanDevice | `bsp/bspCan/` — FDCAN register-level driver (G4) |
| bspCan target | Static lib, conditional on `CAN_TYPE` variable |
| Unit tests (wired) | `BxCanDeviceTest` (~4200 lines), `FdCanDeviceTest` (~4100 lines), `FdCanDeviceFdModeTest` (~1500 lines) |

#### Dependencies

- PR 1: `bspMcu` (register definitions), `CAN_TYPE` variable
- PR 2: `bspIo` (used in some test helpers)

#### CMake registration (required in this PR)

- New `bspCan/test/CMakeLists.txt` defining `BxCanDeviceTest`,
  `FdCanDeviceTest`, `FdCanDeviceFdModeTest` executables
- Root `CMakeLists.txt` adds `add_subdirectory(bspCan/test)` under stm32
  unit test section

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| CAN device tests | `ctest --preset tests-stm32-debug` | 3 new test targets discovered and pass: `BxCanDeviceTest`, `FdCanDeviceTest`, `FdCanDeviceFdModeTest` |
| Prior tests unbroken | | All PR 0-2 tests still pass |
| Formatting | `treefmt` | Clean |

---

### PR 4 — CAN Transceivers (bxCAN + FDCAN) + Unit Tests

**Goal:** Add the transceiver adapters that bridge CAN device drivers to the
openbsw `cpp2can` framework.

#### Scope

| Area | What |
|------|------|
| BxCanTransceiver | `bsp/bxCanTransceiver/` — bxCAN transceiver adapter |
| FdCanTransceiver | `bsp/fdCanTransceiver/` — FDCAN transceiver adapter |
| Unit tests (wired) | `BxCanTransceiverTest` (~1100 lines), `BxCanTransceiverIntegrationTest` (~960 lines), `FdCanTransceiverTest` (~1000 lines), `FdCanTransceiverIntegrationTest` (~960 lines) |

#### Dependencies

- PR 3: `bspCan` (BxCanDevice / FdCanDevice)
- Existing: `cpp2can`, `async`, `lifecycle`

#### CMake registration (required in this PR)

- Fix `bxCanTransceiver/test/CMakeLists.txt` dependencies and ensure it
  defines both `BxCanTransceiverTest` and `BxCanTransceiverIntegrationTest`
  executables
- Fix `fdCanTransceiver/test/CMakeLists.txt` dependencies and ensure it
  defines both `FdCanTransceiverTest` and `FdCanTransceiverIntegrationTest`
  executables
- Root `CMakeLists.txt` adds `add_subdirectory` for both
  `bxCanTransceiver/test` and `fdCanTransceiver/test` under the stm32
  unit test section

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| Transceiver tests | `ctest --preset tests-stm32-debug` | 4 new targets discovered and pass: `BxCanTransceiverTest`, `BxCanTransceiverIntegrationTest`, `FdCanTransceiverTest`, `FdCanTransceiverIntegrationTest` |
| Prior tests unbroken | | All PR 0-3 tests still pass |
| Formatting | `treefmt` | Clean |

---

### PR 5 — Safety: HardFault Handler + IWDG Watchdog

**Goal:** Add fault handling and watchdog safety modules.

#### Scope

| Area | What |
|------|------|
| hardFaultHandler | `platforms/stm32/hardFaultHandler/` — Cortex-M4 HardFault handler (assembly) |
| safeBspMcuWatchdog | `platforms/stm32/safety/safeBspMcuWatchdog/` — IWDG watchdog driver |
| Unit tests (wired) | `WatchdogTest` (~980 lines), `WatchdogStressTest` (~760 lines), `SafetyManagerTest` (~680 lines) |

#### Dependencies

- PR 1: `bspMcu` (register definitions)
- Existing: `safeSupervisor`, `safeUtils`

Note: PR 5 does not depend on PRs 2-4. It could theoretically be reviewed in
parallel with them. Kept in sequence to simplify the review queue.

#### CMake registration (required in this PR)

- New `safeBspMcuWatchdog/test/CMakeLists.txt` defining `WatchdogTest`,
  `WatchdogStressTest`, and `SafetyManagerTest` (all three safety tests
  live under `safeBspMcuWatchdog/test/` in the current tree)
- Root `CMakeLists.txt` adds `add_subdirectory(safeBspMcuWatchdog/test)`
  under stm32 unit test section

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| Safety tests | `ctest --preset tests-stm32-debug` | 3 new targets discovered and pass: `WatchdogTest`, `WatchdogStressTest`, `SafetyManagerTest` |
| Prior tests unbroken | | All PR 0-4 tests still pass |
| Formatting | `treefmt` | Clean |

---

### PR 6 — FreeRTOS Port + NUCLEO-G474RE Board (First Bootable Binary)

**Goal:** Integrate FreeRTOS, create the G474RE board directory, produce the
first cross-compilable firmware image, and make DoCAN/UDS configuration
board-specific.

This is where application presets first appear — because now the scaffolding
exists.

#### Scope

| Area | What |
|------|------|
| FreeRTOS CM4 port | `platforms/stm32/3rdparty/freertos_cm4_sysTick/` — port.c, portmacro.h, LICENSE |
| freeRtosCoreConfiguration | `nucleo_g474re/freeRtosCoreConfiguration/` — `FreeRtosPlatformConfig.h` |
| G474RE board directory | `executables/referenceApp/platforms/nucleo_g474re/` |
| Options.cmake | Sets `OPENBSW_PLATFORM=stm32`, `STM32_CHIP=STM32G474RE`, enables CAN, Transport, UDS |
| Board CMakeLists.txt | Orchestrates: bspConfiguration, freeRtosCoreConfiguration, safety, main |
| bspConfiguration | UART config, stdIo for G474RE |
| main | `main.cpp`, `CanSystem` (FDCAN), `StaticBsp`, ISR overrides, linker script |
| startUp | INTERFACE lib carrying linker script + linking bspMcu, hardFaultHandler |
| osHooks | FreeRTOS OS hooks |
| safeLifecycle | Board-level SafetyManager |
| RTOS aliases | `freeRtosPort` -> `freeRtosCm4SysTickPort`, `freeRtosPortImpl` -> `freeRtosCm4SysTick` |
| Preset | `nucleo-g474re-freertos-gcc` (configure + build) |
| DoCAN config refactor | Extract hardcoded DoCAN addressing from shared `DoCanSystem.cpp` into a board-level config header (e.g. `doCanConfig.h`). Shared code reads from config via `#include`. s32k148evb provides its existing values (`{0x02A, 0x0F0, 0x0F0}`, `LOGICAL_ADDRESS=0x002A`) — zero behavior change. G474RE provides STM32 values (`{0x7E0, 0x7E8, 0x7E8}`, `LOGICAL_ADDRESS=0x0600`). |
| UDS session config | `AppProgrammingSession` class lives in G474RE board `main/` directory (not shared `udsConfiguration/`). Session transitions configured via board-level wiring, not by modifying shared `DiagSession.cpp`. |

#### Shared code changes (additive only, no behavior change)

The following shared files get a **minimal, additive** change to read from
board-level config instead of hardcoding values:

- `application/src/systems/DoCanSystem.cpp` — replace hardcoded addressing
  tuple with `#include "app/doCanConfig.h"` and use config constants
- `configuration/include/app/appConfig.h` — replace hardcoded
  `LOGICAL_ADDRESS` with a board-configurable default

Each existing board (s32k148evb, posix) provides a `doCanConfig.h` that
reproduces the current hardcoded values. **Net behavior change for existing
platforms: zero.**

#### Dependencies

- PR 0-5: all platform + BSP + CAN + safety targets
- Existing: `freeRtos`, `lifecycle`, `configuration`, `async`, `cpp2can`

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| Preset configures | `cmake --preset nucleo-g474re-freertos-gcc` | Success |
| Cross-compile | `cmake --build --preset nucleo-g474re-freertos-gcc` | ELF binary produced |
| Binary size | `arm-none-eabi-size` on output | Fits in G474RE flash (512 KB) |
| Boot smoke (HIL) | Flash ELF to NUCLEO-G474RE, attach UART | Board boots to RTOS scheduler, UART prints startup banner within 2 s |
| s32k148 unbroken | `cmake --preset s32k148-freertos-gcc && build` | Still succeeds (addressing unchanged) |
| s32k148 tests | `ctest --preset tests-s32k1xx-debug` | All pass |
| Unit tests unbroken | `ctest --preset tests-stm32-debug` | All prior tests pass |
| Formatting | `treefmt` | Clean |

Boot smoke is required because this PR introduces startup assembly, clock
init, linker scripts, and RTOS scheduler bring-up — none of which are
exercised by host-side unit tests or cross-compile alone.

---

### PR 7 — ThreadX Port + G474RE ThreadX Variant

**Goal:** Add ThreadX as an alternative RTOS for G474RE.

#### Scope

| Area | What |
|------|------|
| ThreadX CM4 port | `platforms/stm32/3rdparty/threadx/` — assembly port files, tx_port.h, LICENSE |
| threadXCoreConfiguration | `nucleo_g474re/threadXCoreConfiguration/` — `tx_user.h` |
| ThreadX osHooks | `nucleo_g474re/main/src/osHooks/threadx/osHooks.cpp` |
| ThreadX low-level init | `tx_initialize_low_level.S`, `tx_execution_initialize.c` |
| RTOS aliases | `threadXPort` -> `threadXCortexM4Port`, `threadXPortImpl` -> `threadXCortexM4` |
| Preset | `nucleo-g474re-threadx-gcc` |

#### Dependencies

- PR 6: G474RE board directory, CMakeLists.txt with ThreadX conditional
- Existing: `threadX` core library

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| Preset configures | `cmake --preset nucleo-g474re-threadx-gcc` | Success |
| Cross-compile | `cmake --build --preset nucleo-g474re-threadx-gcc` | ELF binary produced |
| Boot smoke (HIL) | Flash ThreadX ELF to NUCLEO-G474RE, attach UART | Board boots to ThreadX scheduler, UART prints startup banner within 2 s |
| FreeRTOS build unbroken | `cmake --build --preset nucleo-g474re-freertos-gcc` | Still succeeds |
| Unit tests unbroken | `ctest --preset tests-stm32-debug` | All prior tests pass |
| Formatting | `treefmt` | Clean |

---

### PR 8 — Second Board: NUCLEO-F413ZH

**Goal:** Add the F413ZH board with bxCAN support. Validates the second CAN
path (bxCAN vs FDCAN) and the second chip family (F4 vs G4).

#### Scope

| Area | What |
|------|------|
| F413ZH board directory | `executables/referenceApp/platforms/nucleo_f413zh/` — full board |
| Options.cmake | Sets `STM32_CHIP=STM32F413ZH`, enables CAN, Transport, UDS |
| bspConfiguration | F413ZH-specific UART config, stdIo |
| freeRtosCoreConfiguration | F413ZH FreeRTOS tuning |
| threadXCoreConfiguration | F413ZH ThreadX config |
| main | F413ZH `main.cpp`, `CanSystem` (bxCAN @ 500 kbit/s), `StaticBsp`, ISR overrides, linker script (1.5 MB Flash, 320 KB SRAM) |
| doCanConfig.h | F413ZH DoCAN addressing (`{0x7E0, 0x7E8, 0x7E8}`) |
| safeLifecycle | F413ZH SafetyManager |
| Presets | `nucleo-f413zh-freertos-gcc`, `nucleo-f413zh-threadx-gcc` |

#### Dependencies

- PR 0-7: all platform, BSP, CAN, safety, RTOS targets
- PR 6: DoCAN config refactor (board-level `doCanConfig.h` pattern)

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| FreeRTOS preset | `cmake --preset nucleo-f413zh-freertos-gcc && build` | ELF produced |
| ThreadX preset | `cmake --preset nucleo-f413zh-threadx-gcc && build` | ELF produced |
| Boot smoke (HIL) | Flash FreeRTOS ELF to NUCLEO-F413ZH, attach UART | Board boots to RTOS scheduler, UART prints startup banner within 2 s |
| G474RE builds unbroken | Both G474RE presets still build | Success |
| s32k148 unbroken | `cmake --preset s32k148-freertos-gcc && build` | Success |
| Unit tests unbroken | `ctest --preset tests-stm32-debug` | All prior tests pass |
| Formatting | `treefmt` | Clean |

---

### PR 9 — UDS Diagnostic Services + Unit Tests

**Goal:** Add 9 UDS diagnostic services for the STM32 platform with full unit
test coverage.

#### Scope

| Service (SID) | Implementation |
|----------------|---------------|
| 0x11 ECUReset + HardReset | Built-in |
| 0x14 ClearDiagnosticInformation | `Stm32ClearDtc` |
| 0x19 ReadDTCInformation | `Stm32ReadDtcInfo` |
| 0x22 ReadDataByIdentifier | 6 new DIDs: F190 (VIN), F195 (SW version), F18C (serial), F193 (HW version), F18A (supplier), F180 (boot SW) |
| 0x27 SecurityAccess | `Stm32SecurityAccess` (seed/key) |
| 0x2E WriteDataByIdentifier | `Stm32WriteVin` (DID F190) |
| 0x31 RoutineControl | `Stm32DemoRoutine` (3 routines: FF00, FF01, FF02) |
| 0x85 ControlDTCSetting | `Stm32ControlDtcSetting` |
| DTC engine | `Stm32DtcManager` (in-memory DTC store) |

All new source files added to `app.referenceApp` under `PLATFORM_SUPPORT_UDS`
guard. No shared application code modified.

#### Unit tests (wired)

| Test | What it covers |
|------|---------------|
| `Stm32DtcManagerTest` | In-memory DTC store: add, clear, read, overflow |
| `Stm32SecurityAccessTest` | Seed/key computation, lock/unlock state machine |
| `Stm32DemoRoutineTest` | Start/stop/results for 3 demo routines |
| `Stm32WriteVinTest` | VIN write to scratchpad, length validation |

These tests use the UDS framework's `AbstractDiagJob` mock dispatcher — no
hardware dependency.

#### Dependencies

- PR 6: DoCAN config, board-level `main.cpp` that instantiates `UdsSystem`
- Existing: UDS framework (`DiagDispatcher`, `DiagJob`, service base classes)

#### CMake registration (required in this PR)

- New `test/CMakeLists.txt` files defining `Stm32DtcManagerTest`,
  `Stm32SecurityAccessTest`, `Stm32DemoRoutineTest`, `Stm32WriteVinTest`
- Root `CMakeLists.txt` adds `add_subdirectory()` for each under stm32
  unit test section

#### Test gate

| Check | Command | Expected |
|-------|---------|----------|
| New UDS unit tests | `ctest --preset tests-stm32-debug` | 4 new targets discovered and pass: `Stm32DtcManagerTest`, `Stm32SecurityAccessTest`, `Stm32DemoRoutineTest`, `Stm32WriteVinTest` |
| All cross-compiles | All 4 board presets + s32k148 build | Success |
| Prior tests unbroken | | All pass |
| Formatting | `treefmt` | Clean |

---

## Dependency Graph

```
PR 0  CMSIS Shared Library (prep refactor)
  |
  v
PR 1  STM32 MCU Foundation
  |
  v
PR 2  BSP Peripheral Drivers + Tests
  |
  v
PR 3  CAN Device Drivers + Tests
  |
  v
PR 4  CAN Transceivers + Tests
  |
  +---------+
  |         |
  v         v
PR 5      (independent of 2-4)
Safety
  |
  v
PR 6  FreeRTOS + G474RE Board      <-- first application preset
  |   + DoCAN config refactor         (additive shared-code change,
  |                                    zero behavior change for s32k148)
  v
PR 7  ThreadX Port
  |
  v
PR 8  F413ZH Board                 <-- second board presets
  |
  v
PR 9  UDS Services + Tests
```

**Critical path:** PR 0 -> PR 1 -> PR 2 -> PR 3 -> PR 4 -> PR 6 -> PR 9

PRs 5 and 7 are off the critical path and can be reviewed in parallel with
their neighbors if the maintainer has bandwidth.

---

## Key Differences from Current State

| Issue | Current (#413-#422) | Proposed (PR 0-9) |
|-------|---------------------|-------------------|
| CMSIS | Duplicated in `platforms/stm32/` | Shared in `libs/3rdparty/cmsis/` via standalone prep PR |
| F413ZH presets | In PR #413 (no platform dir exists) | In PR 8 (with full platform dir) |
| G474RE FreeRTOS preset | In PR #413 (no FreeRTOS config exists) | In PR 6 (with FreeRTOS port + config) |
| G474RE ThreadX preset | In PR #413 (no ThreadX port exists) | In PR 7 (with ThreadX port) |
| BSP unit tests | Source files exist, not wired | All wired in their introducing PR |
| CAN device tests | ~9800 lines of tests, not wired | Wired in PR 3 |
| DoCAN addressing | PR #421 overwrites shared code globally | PR 6 refactors to board-level config (s32k148 unchanged) |
| UDS session changes | PR #421 modifies shared DiagSession.cpp | PR 6 puts AppProgrammingSession in board directory |
| Unrelated commits | Mixed into PR #413 branch | Cleaned out |
| UDS service tests | None | 4 new test suites in PR 9 |
| PR count | 10 PRs (#413-#422) | 10 PRs (PR 0 + PRs 1-9) |

---

## Risks

1. **CMSIS refactor (PR 0)** — Moves files used by s32k1xx. Low risk (include
   path change only, no code change). Mitigated by running full s32k1xx test
   suite + cross-compile as the test gate.

2. **DoCAN config refactor (PR 6)** — Adds a `#include` to shared code and
   requires each board to provide `doCanConfig.h`. Existing boards (s32k148evb,
   posix) must provide headers with their current values. Mitigated by the
   test gate: s32k148 builds + tests must pass with zero behavior change.

3. **RTOS port execution proof (PRs 6, 7, 8)** — FreeRTOS and ThreadX ports
   are vendor assembly code with no unit tests. Cross-compile alone does not
   verify startup, clocks, linker scripts, or interrupt bring-up. A mandatory
   HIL boot-smoke check (flash + UART banner) is included in the test gates
   for PRs 6-8 to close this gap. This requires physical NUCLEO boards.

4. **Branch rebasing** — Restructuring requires rebasing all branches. Each
   branch must be clean (only its own commits) and up to date with current
   `main`. This is a one-time cost.

5. **Reviewer bandwidth** — 10 PRs in sequence means the maintainer is the
   single reviewer bottleneck. Offering PRs 5 and 7 as parallel review
   opportunities helps but doesn't eliminate the constraint.

---

## Proposed Upstream Communication

After internal review, propose this strategy to the maintainer by:

1. Replying to the 3 open comments on PR #413 with a summary:
   - CMSIS: "We will extract CMSIS into `libs/3rdparty/cmsis/` in a separate
     prep PR (PR 0). The STM32 PRs will reference the shared copy."
   - Broken presets: "We are restructuring the series so application presets
     only appear in the PR that provides all required scaffolding. PR 1 will
     have no application presets."
   - G474RE FreeRTOS build: "The FreeRTOS preset and config will be introduced
     together in PR 6."

2. Linking to this strategy document (or a cleaned version)

3. Asking whether the maintainer prefers:
   - (a) The DoCAN config refactor in PR 6, or a separate prep PR
   - (b) Any other grouping preferences

4. Then rebasing and force-pushing the branches to match the agreed strategy
