# STM32 Platform Port — Target Architecture

**Project:** eclipse-openbsw/openbsw
**Author:** nhuvaoanh123 (An Dao)
**Date:** 2026-04-16

> **Note:** This document describes the target architecture after the
> restructured PR series (PR 0-9) is rebased and merged. It does not
> reflect the current state of the `stm32-pr-1` through `stm32-pr-10`
> branches, which still carry the original packaging (duplicated CMSIS,
> premature presets, unwired tests). See `docs/stm32-pr-strategy.md` for
> what changes between the current branches and the target state.

---

## Supported Hardware

| Board | MCU | Core | Clock | Flash | SRAM | CAN | FPU |
|-------|-----|------|-------|-------|------|-----|-----|
| NUCLEO-G474RE | STM32G474RE | Cortex-M4F | 170 MHz | 512 KB | 128 KB | FDCAN (CAN FD) | FPv4-SP |
| NUCLEO-F413ZH | STM32F413ZH | Cortex-M4F | 96 MHz | 1.5 MB | 320 KB | bxCAN (Classic) | FPv4-SP |

Both boards share the same Cortex-M4 CMSIS core (`libs/3rdparty/cmsis/`,
created by PR 0) and the same platform driver layer (`platforms/stm32/`).
Board-specific configuration lives under
`executables/referenceApp/platforms/<board>/`.

---

## Architecture

```
executables/referenceApp/
  platforms/
    nucleo_g474re/          Board-specific: Options, main, CanSystem,
    nucleo_f413zh/          FreeRTOS/ThreadX config, linker script, safety,
                            doCanConfig.h (board-level DoCAN addressing)

platforms/stm32/
  cmake/                    Chip selection (stm32f413zh.cmake, stm32g474re.cmake)
  bsp/
    bspMcu/                 Device headers, startup ASM, mcu.h, CMSIS includes
    bspClock/               Clock config (F4 + G4 variants)
    bspIo/                  GPIO driver
    bspUart/                UART driver
    bspTimer/               System timer
    bspAdc/                 ADC driver
    bspEepromDriver/        Flash EEPROM driver
    bspInterruptsImpl/      Interrupt manager
    bspCan/                 CAN device drivers (bxCAN or FDCAN, compile-time)
    bxCanTransceiver/       bxCAN -> cpp2can adapter
    fdCanTransceiver/       FDCAN -> cpp2can adapter
  3rdparty/
    freertos_cm4_sysTick/   FreeRTOS Cortex-M4 port (SysTick)
    threadx/                ThreadX Cortex-M4 port
  safety/
    safeBspMcuWatchdog/     IWDG watchdog driver
  hardFaultHandler/         Cortex-M4 HardFault handler (assembly)
  etlImpl/                  ETL platform bridge

libs/3rdparty/cmsis/        Shared ARM CMSIS 6.1.0 (used by s32k1xx + stm32)
```

---

## Chip Selection

The cmake variable `STM32_CHIP` (set via CMakePresets or command line) selects
the chip-specific configuration:

| Variable | STM32G474RE | STM32F413ZH |
|----------|-------------|-------------|
| `STM32_FAMILY` | G4 | F4 |
| `CAN_TYPE` | FDCAN | BXCAN |
| Startup ASM | `startup_stm32g474xx.s` | `startup_stm32f413xx.s` |
| Device define | `STM32G474xx` | `STM32F413xx` |

The `CAN_TYPE` variable drives compile-time selection of CAN device drivers:
FDCAN for G4 family, bxCAN for F4 family.

---

## RTOS Support

Both FreeRTOS and ThreadX are supported via a Cortex-M4 SysTick port.
The RTOS is selected at configure time via `BUILD_TARGET_RTOS`:

| RTOS | Port location | Config header |
|------|--------------|---------------|
| FreeRTOS | `platforms/stm32/3rdparty/freertos_cm4_sysTick/` | `<board>/freeRtosCoreConfiguration/` |
| ThreadX | `platforms/stm32/3rdparty/threadx/ports/cortex_m4/gnu/` | `<board>/threadXCoreConfiguration/` |

Each board provides RTOS aliases (`freeRtosPort`/`freeRtosPortImpl` or
`threadXPort`/`threadXPortImpl`) so the framework resolves the correct port.

---

## Boot Sequence

1. **Reset_Handler** (startup ASM): load stack pointer, copy `.data` from
   Flash to SRAM, zero `.bss`
2. **FPU enable**: set CP10/CP11 full access at CPACR (0xE000ED88)
3. **SystemInit()**: vendor clock/PLL configuration (weak symbol)
4. **__libc_init_array()**: C++ static constructors
5. **main()**: RTOS initialization, task creation, scheduler start

---

## CAN Configuration

| Config | NUCLEO-G474RE | NUCLEO-F413ZH |
|--------|---------------|---------------|
| CAN type | FDCAN (Classic + FD frames) | bxCAN (Classic only) |
| Bitrate | Board-configurable | 500 kbit/s |
| DoCAN addressing | `{0x7E0, 0x7E8, 0x7E8}` | `{0x7E0, 0x7E8, 0x7E8}` |
| Logical address | `0x0600` | `0x0600` |
| ISR | FDCAN1_IT0/IT1 (IRQ 21/22) | CAN1_TX/RX0/RX1/SCE (IRQ 19-22) |

In the target architecture, DoCAN addressing is board-specific via
`doCanConfig.h` under each board's `bspConfiguration/` directory. Shared
`DoCanSystem.cpp` reads from this config header instead of hardcoding values.
Existing platforms (s32k148evb, posix) provide headers with their current
values — zero behavior change.

> **Current branch state:** The `stm32-pr-9` branch still hardcodes
> addressing in shared `DoCanSystem.cpp` and `appConfig.h`. The board-level
> `doCanConfig.h` pattern is part of the restructured PR 6 scope.

---

## UDS Diagnostic Services

9 UDS services implemented for the STM32 platform:

| SID | Service | Implementation |
|-----|---------|---------------|
| 0x10 | DiagnosticSessionControl | Framework (existing) |
| 0x11 | ECUReset + HardReset | Built-in |
| 0x14 | ClearDiagnosticInformation | `Stm32ClearDtc` |
| 0x19 | ReadDTCInformation | `Stm32ReadDtcInfo` |
| 0x22 | ReadDataByIdentifier | 6 DIDs (VIN, SW ver, serial, HW ver, supplier, boot SW) |
| 0x27 | SecurityAccess | `Stm32SecurityAccess` (seed/key) |
| 0x2E | WriteDataByIdentifier | `Stm32WriteVin` (DID F190) |
| 0x31 | RoutineControl | `Stm32DemoRoutine` (3 routines) |
| 0x85 | ControlDTCSetting | `Stm32ControlDtcSetting` |

All services are guarded by `PLATFORM_SUPPORT_UDS` and use the existing
openbsw UDS framework (`DiagDispatcher`, `AbstractDiagJob`).

---

## Build Presets

| Preset | Board | RTOS | Purpose |
|--------|-------|------|---------|
| `tests-stm32-debug` | — | — | Unit tests (host build) |
| `nucleo-g474re-freertos-gcc` | G474RE | FreeRTOS | Cross-compile firmware |
| `nucleo-g474re-threadx-gcc` | G474RE | ThreadX | Cross-compile firmware |
| `nucleo-f413zh-freertos-gcc` | F413ZH | FreeRTOS | Cross-compile firmware |
| `nucleo-f413zh-threadx-gcc` | F413ZH | ThreadX | Cross-compile firmware |

Unit test preset runs on host (no ARM toolchain required). Board presets
require `arm-none-eabi-gcc`.

---

## Unit Test Coverage

### Currently wired and registered (stm32-pr-10 branch)

The root `CMakeLists.txt` registers four STM32 test subdirectories under
the `tests-stm32-debug` preset:

| Test directory | Suite | Status |
|---------------|-------|--------|
| `platforms/stm32/unitTest/` | (placeholder, no tests) | Configures only |
| `platforms/stm32/bsp/bxCanTransceiver/test/` | BxCanTransceiverTest | Build fails (missing deps) |
| `platforms/stm32/bsp/fdCanTransceiver/test/` | FdCanTransceiverTest | Build fails (missing deps) |
| `platforms/stm32/bsp/bspUart/test/` | IncludeTest only | Passes (UartTest.cpp not built) |

### Target coverage after rebase (PR 0-9)

Each PR in the restructured series wires its test suites via new
`CMakeLists.txt` entries and root `add_subdirectory()` registrations.
The table below shows intended end-state coverage, not current branch state.

| PR | Area | Test suites | Lines (approx) | CMake registration |
|----|------|------------|----------------|--------------------|
| 2 | GPIO | GpioTest, GpioStressTest | ~2800 | `bspIo/test/CMakeLists.txt` + root `add_subdirectory` |
| 2 | UART | UartTest | ~1000 | `bspUart/test/CMakeLists.txt` updated to build UartTest.cpp |
| 2 | ADC | AdcTest, AdcExtendedTest | ~2100 | `bspAdc/test/CMakeLists.txt` + root `add_subdirectory` |
| 2 | EEPROM | FlashEepromDriverTest, FlashEepromStressTest | ~2900 | `bspEepromDriver/test/CMakeLists.txt` + root `add_subdirectory` |
| 2 | Clock | ClockConfigTest | ~900 | `bspClock/test/CMakeLists.txt` + root `add_subdirectory` |
| 3 | bxCAN device | BxCanDeviceTest | ~4200 | `bspCan/test/CMakeLists.txt` + root `add_subdirectory` |
| 3 | FDCAN device | FdCanDeviceTest, FdCanDeviceFdModeTest | ~5600 | `bspCan/test/CMakeLists.txt` + root `add_subdirectory` |
| 4 | bxCAN transceiver | BxCanTransceiverTest, IntegrationTest | ~2100 | `bxCanTransceiver/test/CMakeLists.txt` + root `add_subdirectory` |
| 4 | FDCAN transceiver | FdCanTransceiverTest, IntegrationTest | ~2000 | `fdCanTransceiver/test/CMakeLists.txt` + root `add_subdirectory` |
| 5 | Watchdog | WatchdogTest, WatchdogStressTest | ~1700 | `safeBspMcuWatchdog/test/CMakeLists.txt` + root `add_subdirectory` |
| 5 | SafetyManager | SafetyManagerTest | ~680 | `safeBspMcuWatchdog/test/CMakeLists.txt` (same dir as watchdog tests) |
| 9 | UDS DTC manager | Stm32DtcManagerTest | new | new `test/CMakeLists.txt` + root `add_subdirectory` |
| 9 | UDS SecurityAccess | Stm32SecurityAccessTest | new | new `test/CMakeLists.txt` + root `add_subdirectory` |
| 9 | UDS DemoRoutine | Stm32DemoRoutineTest | new | new `test/CMakeLists.txt` + root `add_subdirectory` |
| 9 | UDS WriteVin | Stm32WriteVinTest | new | new `test/CMakeLists.txt` + root `add_subdirectory` |

All tests run on host with mocked hardware registers — no target board
required. Tests use Google Test / Google Mock.
