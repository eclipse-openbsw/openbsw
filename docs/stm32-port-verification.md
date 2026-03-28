# STM32 Platform Port Verification Plan

**Commit under review:** `25e8dee6` — *Add STM32 platform support (NUCLEO-F413ZH + NUCLEO-G474RE)*
**Date:** 2026-03-27
**Reference implementation:** S32K1xx platform (commit `49933365` and earlier)
**Scope:** 197 files, ~63 196 lines added across `platforms/stm32/`, `executables/referenceApp/platforms/nucleo_*`, top-level CMake, and documentation.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Structural Comparison: STM32 vs S32K](#2-structural-comparison-stm32-vs-s32k)
3. [Verification Areas](#3-verification-areas)
4. [Detailed Checklists](#4-detailed-checklists)
5. [Task Assignments](#5-task-assignments)
6. [Test Matrix](#6-test-matrix)
7. [Risk Register](#7-risk-register)
8. [Sign-off Criteria](#8-sign-off-criteria)

---

## 1. Executive Summary

This verification plan covers the STM32 platform port that adds two NUCLEO development boards to Eclipse OpenBSW:

| Board | MCU | Core | Clock | Flash / RAM | CAN |
|-------|-----|------|-------|-------------|-----|
| NUCLEO-F413ZH | STM32F413ZH | Cortex-M4 | 96 MHz | 1.5 MB / 320 KB | bxCAN (CAN 2.0B) |
| NUCLEO-G474RE | STM32G474RE | Cortex-M4F | 170 MHz | 512 KB / 128 KB | FDCAN (CAN FD) |

The port follows the same register-level, no-vendor-HAL design philosophy as S32K1xx. Verification covers: architecture conformance, register-level correctness, build system integration, unit test coverage, RTOS integration, documentation, and on-target validation.

---

## 2. Structural Comparison: STM32 vs S32K

### 2.1 Directory Layout Alignment

| Component | S32K1xx Path | STM32 Path | Status |
|-----------|-------------|------------|--------|
| Platform root CMake | `platforms/s32k1xx/CMakeLists.txt` | `platforms/stm32/CMakeLists.txt` | Aligned |
| BSP root CMake | `platforms/s32k1xx/bsp/CMakeLists.txt` | `platforms/stm32/bsp/CMakeLists.txt` | Aligned |
| CAN device driver | `bsp/bspFlexCan/` | `bsp/bspCan/` (BxCanDevice + FdCanDevice) | Adapted |
| CAN transceiver | `bsp/canflex2Transceiver/` | `bsp/bxCanTransceiver/` + `bsp/fdCanTransceiver/` | Adapted |
| Clock config | `bsp/bspClock/` | `bsp/bspClock/` (F4 + G4 variants) | Aligned |
| Interrupt impl | `bsp/bspInterruptsImpl/` | `bsp/bspInterruptsImpl/` | Aligned |
| MCU headers | `bsp/bspMcu/` (NXP S32K148.h) | `bsp/bspMcu/` (ST stm32f413xx.h, stm32g474xx.h) | Aligned |
| Timer | `bsp/bspTimer/` (via bspConfiguration) | `bsp/bspTimer/` (DWT CYCCNT) | Adapted |
| UART | `bsp/bspUart/` | `bsp/bspUart/` | Aligned |
| Hard fault handler | `safety/` modules | `hardFaultHandler/` | Adapted |
| FreeRTOS port | `3rdparty/freertos_cm4_sysTick/` | `3rdparty/freertos_cm4_sysTick/` | Aligned |
| ThreadX port | `3rdparty/threadx/ports/cortex_m4/gnu/` | `3rdparty/threadx/ports/cortex_m4/gnu/` | Aligned |
| ETL impl | (in bspConfiguration) | `etlImpl/` (separate module) | Adapted |
| Chip-specific cmake | (inline in CMakeLists.txt) | `cmake/stm32f413zh.cmake`, `cmake/stm32g474re.cmake` | Improved |

### 2.2 Reference App Layout Alignment

| Component | S32K (s32k148evb) | STM32 (nucleo_f413zh / nucleo_g474re) | Status |
|-----------|-------------------|---------------------------------------|--------|
| Board root CMake | `CMakeLists.txt` | `CMakeLists.txt` | Aligned |
| Options.cmake | `Options.cmake` | `Options.cmake` | Aligned |
| bspConfiguration | Full (ADC, CAN, IO, PWM, UART, Timer) | Minimal (UART only) | Intentional |
| Main CMakeLists | `main/CMakeLists.txt` | `main/CMakeLists.txt` | Aligned |
| main.cpp | `main/src/main.cpp` | `main/src/main.cpp` | Adapted |
| CanSystem | `main/src/systems/CanSystem.cpp` | `main/src/systems/CanSystem.cpp` | Adapted |
| BspSystem | `main/src/systems/BspSystem.cpp` | (not present — simplified) | Intentional |
| EthernetSystem | `main/src/systems/S32K148EvbEthernetSystem.cpp` | (not present — no ENET) | Intentional |
| ISR trampolines | `main/src/os/isr/isr_can.cpp` + 5 others | `main/src/os/isr/isr_can.cpp` only | Intentional |
| SafetyManager | Full (IsrHooks, IsrWrapper, SafeIo, SafeMemory) | Stub (init/run/shutdown/cyclic) | Intentional |
| FreeRTOS config | `freeRtosCoreConfiguration/` | `freeRtosCoreConfiguration/` | Aligned |
| ThreadX config | `threadXCoreConfiguration/` | `threadXCoreConfiguration/` | Aligned |
| Linker script | `application.dld` | `application.ld` | Adapted |
| Startup assembly | `startUp.S` + `bootHeader.S` | (in platform bspMcu) | Adapted |
| TX low-level init | `bsp/threadx/tx_initialize_low_level.S` | `bsp/threadx/tx_initialize_low_level.S` | Aligned |

### 2.3 Key Design Differences (Intentional)

1. **CAN transceiver simplification**: STM32 BxCanTransceiver/FdCanTransceiver do not depend on `CanPhy` or `IEcuPowerStateController` (NUCLEO boards have no external CAN PHY). This is intentional.
2. **No BspSystem**: STM32 boards lack ADC, PWM, and IO subsystems, so BspSystem is omitted.
3. **No Ethernet**: STM32F413 and G474 lack ENET peripheral.
4. **Simplified safety**: Stub SafetyManager with sequence monitoring only (no WDOG driver, no memory protection). The STM32 uses IWDG directly in OS hooks.
5. **Separate chip configs**: STM32 extracts chip-specific variables into `cmake/stm32f413zh.cmake` and `cmake/stm32g474re.cmake` — an improvement over S32K's inline approach.
6. **DWT-based timer**: STM32 uses Cortex-M4 DWT CYCCNT (architecture-standard) instead of chip-specific FTM/LPIT timer.

---

## 3. Verification Areas

### Area 1: Build System Integration
### Area 2: MCU Core & Headers
### Area 3: Clock Configuration
### Area 4: Linker Scripts & Startup Assembly
### Area 5: Interrupt Management
### Area 6: UART Driver
### Area 7: CAN Device Drivers (BxCanDevice + FdCanDevice)
### Area 8: CAN Transceivers (BxCanTransceiver + FdCanTransceiver)
### Area 9: System Timer
### Area 10: RTOS Integration (FreeRTOS + ThreadX)
### Area 11: Reference Application
### Area 12: Safety & Hard Fault Handler
### Area 13: Unit Tests
### Area 14: Documentation
### Area 15: Code Quality & Standards

---

## 4. Detailed Checklists

### 4.1 Build System Integration

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| B-1 | `tests-stm32-debug` preset configures and builds | `CMakePresets.json` | [ ] |
| B-2 | `nucleo-f413zh-freertos-gcc` preset configures (requires ARM toolchain) | `CMakePresets.json` | [ ] |
| B-3 | `nucleo-g474re-freertos-gcc` preset configures (requires ARM toolchain) | `CMakePresets.json` | [ ] |
| B-4 | Top-level `CMakeLists.txt` routes `NUCLEO_F413ZH` / `NUCLEO_G474RE` to `platforms/stm32` | `CMakeLists.txt:125-134` | [ ] |
| B-5 | Rust target triple set to `thumbv7em-none-eabihf` for STM32 platforms | `CMakeLists.txt:96-101` | [ ] |
| B-6 | Unit test config adds `bxCanTransceiver/test`, `fdCanTransceiver/test`, `bspUart/test` | `CMakeLists.txt:208-214` | [ ] |
| B-7 | `platforms/stm32/CMakeLists.txt` selects chip config file based on `STM32_CHIP` | `platforms/stm32/CMakeLists.txt` | [ ] |
| B-8 | `platforms/stm32/bsp/CMakeLists.txt` creates `socBsp` target correctly | `platforms/stm32/bsp/CMakeLists.txt` | [ ] |
| B-9 | CAN_TYPE selection (BXCAN/FDCAN) routes to correct transceiver module | `platforms/stm32/bsp/CMakeLists.txt` | [ ] |
| B-10 | Unit test builds skip hardware-dependent modules (startup, clock, MCU) | `platforms/stm32/CMakeLists.txt` | [ ] |
| B-11 | Build and test presets have corresponding configure/build/test entries | `CMakePresets.json` | [ ] |
| B-12 | `--whole-archive` linker trick used for ISR overrides in both boards | `nucleo_f413zh/main/CMakeLists.txt`, `nucleo_g474re/main/CMakeLists.txt` | [ ] |
| B-13 | ThreadX startup sources added as INTERFACE on startUp target | Both main `CMakeLists.txt` | [ ] |
| B-14 | All `module.spec` files have correct format | All `module.spec` files | [ ] |

### 4.2 MCU Core & Headers

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| M-1 | `mcu.h` conditionally includes correct ST device header (`stm32f413xx.h` or `stm32g474xx.h`) | `bspMcu/include/mcu/mcu.h` | [ ] |
| M-2 | CMSIS `core_cm4.h` included with MPU guard (matches S32K pattern) | `bspMcu/include/mcu/mcu.h` | [ ] |
| M-3 | `ENABLE_INTERRUPTS()` / `DISABLE_INTERRUPTS()` macros defined | `bspMcu/include/mcu/mcu.h` | [ ] |
| M-4 | `FEATURE_NVIC_PRIO_BITS` = 4 (correct for both STM32F4 and G4) | `bspMcu/include/mcu/mcu.h` | [ ] |
| M-5 | `typedefs.h` provides `SYS_SetPriority`, `SYS_EnableIRQ`, `SYS_DisableIRQ` wrappers | `bspMcu/include/mcu/typedefs.h` | [ ] |
| M-6 | `softwareSystemReset.h` declares `softwareSystemReset()` (matches S32K interface) | `bspMcu/include/reset/softwareSystemReset.h` | [ ] |
| M-7 | `softwareSystemReset.cpp` calls `__disable_irq()` + `NVIC_SystemReset()` | `bspMcu/src/reset/softwareSystemReset.cpp` | [ ] |
| M-8 | Weak `preResetDiag()` hook provided for platform override | `bspMcu/src/reset/softwareSystemReset.cpp` | [ ] |
| M-9 | CMSIS headers are from official ARM release (check LICENSE) | `bspMcu/include/3rdparty/cmsis/LICENSE` | [ ] |
| M-10 | ST device headers match reference manual register definitions | `stm32f413xx.h`, `stm32g474xx.h` | [ ] |
| M-11 | No `[[noreturn]]` on `softwareSystemReset()` (S32K has it, STM32 doesn't — verify intent) | Compare headers | [ ] |

### 4.3 Clock Configuration

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| C-1 | `configurePll()` declared in `clockConfig.h` (C linkage) | `bspClock/include/clock/clockConfig.h` | [ ] |
| C-2 | F4 clock: HSI 16 MHz or HSE 8 MHz bypass -> PLL -> 96 MHz SYSCLK | `bspClock/src/clockConfig_f4.cpp` | [ ] |
| C-3 | F4 APB1 prescaler: SYSCLK/2 = 48 MHz (bxCAN runs off APB1) | `bspClock/src/clockConfig_f4.cpp` | [ ] |
| C-4 | F4 Flash latency: 3 WS for 96 MHz at 3.3V (RM0430 Table 6) | `bspClock/src/clockConfig_f4.cpp` | [ ] |
| C-5 | G4 clock: HSI 16 MHz -> PLL -> 170 MHz SYSCLK (boost mode) | `bspClock/src/clockConfig_g4.cpp` | [ ] |
| C-6 | G4 APB1 prescaler: verify FDCAN clock derivation (PCLK1 or PLL Q) | `bspClock/src/clockConfig_g4.cpp` | [ ] |
| C-7 | G4 Flash latency: 4 WS for 170 MHz (RM0440 Table 9) | `bspClock/src/clockConfig_g4.cpp` | [ ] |
| C-8 | G4 voltage range: Range 1 boost required for 170 MHz | `bspClock/src/clockConfig_g4.cpp` | [ ] |
| C-9 | PLL lock timeout handled (not infinite loop) | Both clock files | [ ] |
| C-10 | No floating point used in clock config (pre-FPU init context) | Both clock files | [ ] |

### 4.4 Linker Scripts & Startup Assembly

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| L-1 | F4 FLASH origin = 0x08000000, length = 1536K (1.5 MB) | `STM32F413ZHxx_FLASH.ld` | [ ] |
| L-2 | F4 RAM origin = 0x20000000, length = 320K | `STM32F413ZHxx_FLASH.ld` | [ ] |
| L-3 | G4 FLASH origin = 0x08000000, length = 512K | `STM32G474RExx_FLASH.ld` | [ ] |
| L-4 | G4 RAM origin = 0x20000000, length = 128K | `STM32G474RExx_FLASH.ld` | [ ] |
| L-5 | `.isr_vector` placed at FLASH start, KEEP'd | Both linker scripts | [ ] |
| L-6 | `.data` section copied from FLASH (LMA) to RAM (VMA) | Both linker scripts | [ ] |
| L-7 | `.bss` section zero-filled | Both linker scripts | [ ] |
| L-8 | `_estack` symbol defined at top of RAM | Both linker scripts | [ ] |
| L-9 | Heap/stack size: min 512 / 1024 bytes with overflow check | Both linker scripts | [ ] |
| L-10 | `._user_heap_stack` section has ASSERT for minimum sizes | Both linker scripts | [ ] |
| L-11 | Startup: `Reset_Handler` sets SP, copies .data, zeros .bss, calls `SystemInit`, calls `main` | Both startup `.s` files | [ ] |
| L-12 | Startup: `Default_Handler` is infinite loop (safe for unhandled IRQs) | Both startup `.s` files | [ ] |
| L-13 | F4 vector table: all 102 IRQs listed (match RM0430) | `startup_stm32f413xx.s` | [ ] |
| L-14 | G4 vector table: all IRQs listed (match RM0440) | `startup_stm32g474xx.s` | [ ] |
| L-15 | Startup uses `.syntax unified`, `.cpu cortex-m4`, `.thumb` | Both startup files | [ ] |
| L-16 | No `.noinit` section defined in linker (hard fault handler uses hardcoded addresses) | Both linker scripts | [ ] |
| L-17 | Application linker script (board-level) references correct platform linker script | Board main/CMakeLists.txt | [ ] |

### 4.5 Interrupt Management

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| I-1 | `disableAllInterrupts()` uses CPSID I + ISB + DSB + DMB | `disableEnableAllInterrupts.h` | [ ] |
| I-2 | `enableAllInterrupts()` uses ISB + DSB + DMB + CPSIE I | `disableEnableAllInterrupts.h` | [ ] |
| I-3 | Memory barriers in correct order (barrier before enable, after disable) | `disableEnableAllInterrupts.h` | [ ] |
| I-4 | FreeRTOS variant: `suspendResumeAllInterrupts.h` uses BASEPRI | `bspInterruptsImpl/freertos/` | [ ] |
| I-5 | ThreadX variant: `suspendResumeAllInterrupts.h` uses BASEPRI | `bspInterruptsImpl/threadx/` | [ ] |
| I-6 | `interrupt_manager.c` wraps CMSIS NVIC functions (same as S32K) | `bspInterruptsImpl/src/interrupt_manager.c` | [ ] |
| I-7 | Include guard for unit test builds (no inline asm on host) | `disableEnableAllInterrupts.h` | [ ] |

### 4.6 UART Driver

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| U-1 | `Uart` class provides `init()`, `write()`, `read()`, `waitForTxReady()` | `bspUart/include/bsp/Uart.h` | [ ] |
| U-2 | Singleton pattern via `getInstance(Id)` | `bspUart/include/bsp/Uart.h` | [ ] |
| U-3 | F4 UART uses SR/DR register pair (USARTv1) | `bspUart/src/Uart.cpp` | [ ] |
| U-4 | G4 UART uses ISR/TDR/RDR register pair (USARTv2) | `bspUart/src/Uart.cpp` | [ ] |
| U-5 | Conditional compilation for F4 vs G4 register differences | `bspUart/src/Uart.cpp` | [ ] |
| U-6 | Baud rate calculation correct for each chip clock | Board `UartConfig.cpp` | [ ] |
| U-7 | GPIO AF mapping matches reference manual pinout | Board `UartConfig.cpp` | [ ] |
| U-8 | UART include test passes | `bspUart/test/src/IncludeTest.cpp` | [ ] |

### 4.7 CAN Device Drivers

#### 4.7.1 BxCanDevice (STM32F4 bxCAN)

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| CAN-B1 | `init()` enables RCC clock for CAN1 peripheral | `BxCanDevice.cpp` | [ ] |
| CAN-B2 | GPIO configured: TX = push-pull AF, RX = pull-up AF | `BxCanDevice.cpp` | [ ] |
| CAN-B3 | Init mode entry: set INRQ, wait for INAK | `BxCanDevice.cpp` | [ ] |
| CAN-B4 | Bit timing: prescaler, BS1, BS2, SJW from Config struct | `BxCanDevice.cpp` | [ ] |
| CAN-B5 | 500 kbps timing correct: 48 MHz APB1 / prescaler / (1+BS1+BS2) = 500k | `CanSystem.cpp` (F413ZH) | [ ] |
| CAN-B6 | Accept-all filter: 32-bit mask mode, mask=0, id=0 | `BxCanDevice.cpp` | [ ] |
| CAN-B7 | Filter list mode: 32-bit identifier-list (2 IDs per bank) | `BxCanDevice.cpp` | [ ] |
| CAN-B8 | TX path: writes to mailboxes 0-2, enables TMEIE | `BxCanDevice.cpp` | [ ] |
| CAN-B9 | RX ISR: drains FIFO0, software filter, enqueue to circular buffer | `BxCanDevice.cpp` | [ ] |
| CAN-B10 | TX ISR: clears RQCP, disables TMEIE when idle | `BxCanDevice.cpp` | [ ] |
| CAN-B11 | Bus-off detection via ESR BOFF bit | `BxCanDevice.cpp` | [ ] |
| CAN-B12 | Error counters read from CAN->ESR (TEC/REC) | `BxCanDevice.cpp` | [ ] |
| CAN-B13 | RX interrupt masking (FMPIE0) for critical sections | `BxCanDevice.cpp` | [ ] |
| CAN-B14 | Software RX queue size = 32 frames | `BxCanDevice.h` | [ ] |
| CAN-B15 | No raw pointer arithmetic on hardware registers (use struct overlay) | `BxCanDevice.cpp` | [ ] |

#### 4.7.2 FdCanDevice (STM32G4 FDCAN)

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| CAN-F1 | `init()` enables RCC clock for FDCAN peripheral | `FdCanDevice.cpp` | [ ] |
| CAN-F2 | GPIO configured: TX = push-pull AF, RX = pull-up AF | `FdCanDevice.cpp` | [ ] |
| CAN-F3 | Init mode: set INIT + CCE, wait for INIT ack | `FdCanDevice.cpp` | [ ] |
| CAN-F4 | Message RAM layout: correct offsets for FDCAN1 (standard/ext filters, RX FIFO, TX FIFO) | `FdCanDevice.h`, `FdCanDevice.cpp` | [ ] |
| CAN-F5 | SRAMCAN base address correct (0x4000A400 for STM32G4) | `FdCanDevice.h` | [ ] |
| CAN-F6 | 500 kbps timing correct: clock / prescaler / (1+TSEG1+TSEG2) = 500k | `CanSystem.cpp` (G474RE) | [ ] |
| CAN-F7 | Accept-all filter via global filter config (ANFS/ANFE = accept) | `FdCanDevice.cpp` | [ ] |
| CAN-F8 | Filter list mode: standard ID exact-match elements | `FdCanDevice.cpp` | [ ] |
| CAN-F9 | TX path: writes to TX FIFO in message RAM | `FdCanDevice.cpp` | [ ] |
| CAN-F10 | RX ISR: snapshot drain of RX FIFO0, software filter, enqueue | `FdCanDevice.cpp` | [ ] |
| CAN-F11 | TX ISR: clears TX complete flag (TCF in IR) | `FdCanDevice.cpp` | [ ] |
| CAN-F12 | Bus-off detection via PSR.BO | `FdCanDevice.cpp` | [ ] |
| CAN-F13 | Error counters from ECR (TEC/REC) | `FdCanDevice.cpp` | [ ] |
| CAN-F14 | Interrupt routing: ILS register routes TX to line 1, RX to line 0 | `FdCanDevice.cpp` | [ ] |
| CAN-F15 | ILE register enables both interrupt lines | `FdCanDevice.cpp` | [ ] |
| CAN-F16 | Software RX queue size = 32 frames | `FdCanDevice.h` | [ ] |
| CAN-F17 | Classic CAN mode (no BRS, no FDF in TX element) | `FdCanDevice.cpp` | [ ] |

### 4.8 CAN Transceivers

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| T-1 | BxCanTransceiver extends `AbstractCANTransceiver` (same as S32K's CanFlex2Transceiver) | `BxCanTransceiver.h` | [ ] |
| T-2 | FdCanTransceiver extends `AbstractCANTransceiver` | `FdCanTransceiver.h` | [ ] |
| T-3 | State machine: CLOSED -> INITIALIZED -> OPEN -> CLOSED (lifecycle transitions) | Both transceivers | [ ] |
| T-4 | `write()` without listener: synchronous `notifySentListeners()` | Both transceivers | [ ] |
| T-5 | `write()` with listener: async callback via TX ISR -> `canFrameSentAsyncCallback()` | Both transceivers | [ ] |
| T-6 | TX queue depth = 3 (matches S32K design) | Both transceivers | [ ] |
| T-7 | `mute()` / `unmute()` state handling | Both transceivers | [ ] |
| T-8 | Static array of transceiver pointers (3 max) for ISR dispatch | Both transceivers | [ ] |
| T-9 | `receiveInterrupt()` / `transmitInterrupt()` static methods for ISR routing | Both transceivers | [ ] |
| T-10 | `disableRxInterrupt()` / `enableRxInterrupt()` static methods | Both transceivers | [ ] |
| T-11 | No CanPhy dependency (intentional difference from S32K) | Both transceivers | [ ] |
| T-12 | No IEcuPowerStateController dependency (intentional difference from S32K) | Both transceivers | [ ] |
| T-13 | `getBaudrate()` returns configured baud rate | Both transceivers | [ ] |
| T-14 | `getHwQueueTimeout()` returns reasonable value | Both transceivers | [ ] |

### 4.9 System Timer

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| ST-1 | Uses DWT CYCCNT (Cortex-M4 standard, chip-agnostic) | `SystemTimer.cpp` | [ ] |
| ST-2 | `initSystemTimer()` enables DWT counter (DEMCR + DWT->CYCCNT) | `SystemTimer.cpp` | [ ] |
| ST-3 | DWT frequency: F4=96 MHz, G4=170 MHz (conditional compilation) | `SystemTimer.cpp` | [ ] |
| ST-4 | Tick frequency: 1 MHz (1 tick = 1 microsecond) | `SystemTimer.cpp` | [ ] |
| ST-5 | 64-bit tick counter with overflow handling | `SystemTimer.cpp` | [ ] |
| ST-6 | `getSystemTimeNs/Us/Ms()` correct conversion | `SystemTimer.cpp` | [ ] |
| ST-7 | Thread-safe tick update with CPSID/CPSIE + barriers | `SystemTimer.cpp` | [ ] |
| ST-8 | `sysDelayUs()` busy-wait implementation correct | `SystemTimer.cpp` | [ ] |
| ST-9 | ETL clock implementations use SystemTimer API | `etlImpl/src/clocks.cpp` | [ ] |

### 4.10 RTOS Integration

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| R-1 | FreeRTOS `FreeRtosPlatformConfig.h`: `configCPU_CLOCK_HZ` matches chip clock | Both boards | [ ] |
| R-2 | FreeRTOS `configTICK_RATE_HZ` = 1000 (1 ms tick) | Both boards | [ ] |
| R-3 | FreeRTOS `configMAX_SYSCALL_INTERRUPT_PRIORITY` compatible with CAN ISR priorities | Both boards | [ ] |
| R-4 | FreeRTOS stack overflow hook: fail-closed (infinite loop) | Both boards osHooks | [ ] |
| R-5 | FreeRTOS malloc failed hook: fail-closed | Both boards osHooks | [ ] |
| R-6 | FreeRTOS tick hook: feeds IWDG (`IWDG->KR = 0xAAAAU`) | Both boards osHooks | [ ] |
| R-7 | ThreadX `tx_user.h`: stack size, priority definitions reasonable | Both boards | [ ] |
| R-8 | ThreadX `tx_initialize_low_level.S`: SysTick config, vector table relocation | Both boards | [ ] |
| R-9 | ThreadX execution hooks: `_tx_execution_thread_enter/exit` map to async tasks | Both boards | [ ] |
| R-10 | ThreadX idle: feeds IWDG in `tx_low_power_enter()` | Both boards | [ ] |
| R-11 | FreeRTOS CM4 SysTick port: `port.c` and `portmacro.h` from upstream | `3rdparty/freertos_cm4_sysTick/` | [ ] |
| R-12 | ThreadX CM4 port: all assembly files from upstream | `3rdparty/threadx/ports/cortex_m4/gnu/` | [ ] |

### 4.11 Reference Application

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| A-1 | F413ZH `SystemInit()`: PLL config, LED init (PB0), USART3 setup (PD8 AF7, 115200) | `nucleo_f413zh/main/src/main.cpp` | [ ] |
| A-2 | G474RE `SystemInit()`: PLL config, LED init (PA5), USART2 setup (PA2 AF7, 115200) | `nucleo_g474re/main/src/main.cpp` | [ ] |
| A-3 | F413ZH BRR calculation for 115200 baud at 48 MHz APB1 (BRR=0x1A1) | `nucleo_f413zh/main/src/main.cpp` | [ ] |
| A-4 | G474RE BRR calculation for 115200 baud at 170 MHz (BRR=1476) | `nucleo_g474re/main/src/main.cpp` | [ ] |
| A-5 | Reset cause decoding from RCC_CSR | `nucleo_f413zh/main/src/main.cpp` | [ ] |
| A-6 | HardFault_Handler: feeds IWDG, dumps registers to UART, blinks LED | Both boards | [ ] |
| A-7 | `platformLifecycleAdd()` adds CanSystem at level 2 | Both boards | [ ] |
| A-8 | `getCanSystem()` returns singleton instance | Both boards | [ ] |
| A-9 | CAN ISR trampolines: RX disables interrupt, holds async lock, dispatches task | Both boards isr_can.cpp | [ ] |
| A-10 | CAN ISR priorities: >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY | Both boards CanSystem.cpp | [ ] |
| A-11 | F413ZH CanSystem: NVIC enables CAN1_RX0_IRQn and CAN1_TX_IRQn | `nucleo_f413zh/main/src/systems/CanSystem.cpp` | [ ] |
| A-12 | G474RE CanSystem: NVIC enables FDCAN1_IT0_IRQn and FDCAN1_IT1_IRQn | `nucleo_g474re/main/src/systems/CanSystem.cpp` | [ ] |
| A-13 | G474RE combined ISR: checks IR register bits for RX (RF0N) and TX (TEFN) | `nucleo_g474re/main/src/systems/CanSystem.cpp` | [ ] |
| A-14 | G474RE defensive IE restore in ISR (re-enables RF0NE + TEFNE if corrupted) | `nucleo_g474re/main/src/systems/CanSystem.cpp` | [ ] |
| A-15 | StaticBsp: same interface as S32K (`init()` method) | Both boards | [ ] |

### 4.12 Safety & Hard Fault Handler

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| S-1 | Hard fault handler: saves R4-R11 before using them as scratch | `hardFaultHandler.s` | [ ] |
| S-2 | Correctly determines active SP (MSP vs PSP) via LR bit 2 | `hardFaultHandler.s` | [ ] |
| S-3 | Dump location: F4 uses 0x2004FC00, G4 uses 0x2001FC00 (top of SRAM) | `hardFaultHandler.s` | [ ] |
| S-4 | Dump size: 0x140 bytes (all regs + stack snapshot + checksum) | `hardFaultHandler.s` | [ ] |
| S-5 | Checksum: sum of all dump words stored at end | `hardFaultHandler.s` | [ ] |
| S-6 | Branches to `HardFault_Handler_Final` (defined in main.cpp) | `hardFaultHandler.s` | [ ] |
| S-7 | SafetyManager stub: init/run/shutdown/cyclic present | Both boards `SafetyManager.cpp` | [ ] |
| S-8 | SafetyManager cyclic: hits sequence monitor (ENTER/LEAVE) | Both boards `SafetyManager.cpp` | [ ] |
| S-9 | IWDG feeding in FreeRTOS tick hook (not just in idle) | Both boards osHooks | [ ] |

### 4.13 Unit Tests

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| UT-1 | BxCanDevice tests: fake hardware structs allocated in static RAM | `BxCanDeviceTest.cpp` | [ ] |
| UT-2 | FdCanDevice tests: fake FDCAN + message RAM in static RAM | `FdCanDeviceTest.cpp` | [ ] |
| UT-3 | BxCanTransceiver tests: GMock-based device mock | `BxCanTransceiverTest.cpp` | [ ] |
| UT-4 | FdCanTransceiver tests: GMock-based device mock | `FdCanTransceiverTest.cpp` | [ ] |
| UT-5 | Integration tests: DoCAN/UDS patterns verified | Both integration test files | [ ] |
| UT-6 | Mock BxCanDevice matches production interface | `test/mock/include/can/BxCanDevice.h` | [ ] |
| UT-7 | Mock FdCanDevice matches production interface | `test/mock/include/can/FdCanDevice.h` | [ ] |
| UT-8 | All 152 tests pass on host with GCC (as claimed in commit message) | Run `ctest --preset tests-stm32-debug` | [ ] |
| UT-9 | No test leaks (valgrind/ASan clean) | Run with sanitizers | [ ] |
| UT-10 | State machine transitions covered: CLOSED->INIT->OPEN->CLOSE->SHUTDOWN | Both transceiver tests | [ ] |
| UT-11 | Error paths tested: double-init, write-when-closed, invalid index | Both transceiver tests | [ ] |
| UT-12 | TX queue overflow behavior tested | Both transceiver tests | [ ] |
| UT-13 | RX queue overflow behavior tested | Both device tests | [ ] |
| UT-14 | Bus-off detection tested | Both device tests | [ ] |
| UT-15 | UART include test passes | `bspUart/test/src/IncludeTest.cpp` | [ ] |

### 4.14 Documentation

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| D-1 | `doc/dev/index.rst` updated to include STM32 platform and modules | `doc/dev/index.rst` | [ ] |
| D-2 | `doc/dev/modules/stm32.rst` lists all BSP modules | `doc/dev/modules/stm32.rst` | [ ] |
| D-3 | `doc/dev/modules/executables.rst` lists both NUCLEO boards | `doc/dev/modules/executables.rst` | [ ] |
| D-4 | `doc/dev/platforms/stm32/index.rst` overview is accurate | `doc/dev/platforms/stm32/index.rst` | [ ] |
| D-5 | `doc/platforms/nucleo_f413zh/index.rst` has correct specs and pinout | `doc/platforms/nucleo_f413zh/index.rst` | [ ] |
| D-6 | `doc/platforms/nucleo_g474re/index.rst` has correct specs and pinout | `doc/platforms/nucleo_g474re/index.rst` | [ ] |
| D-7 | All BSP module `doc/index.rst` files build without Sphinx errors | All module docs | [ ] |
| D-8 | Reference manual links are valid and point to correct documents | Platform overview RST | [ ] |
| D-9 | Test count in overview (46 unit tests) matches reality | `doc/dev/platforms/stm32/index.rst` | [ ] |
| D-10 | All new RST files reachable from root toctree | `doc/dev/index.rst` | [ ] |
| D-11 | Board-specific documentation: bspConfiguration, main, safety modules | All board doc dirs | [ ] |

### 4.15 Code Quality & Standards

| # | Check | File(s) | Verdict |
|---|-------|---------|---------|
| Q-1 | Copyright headers: `// Copyright 2024 Contributors to the Eclipse Foundation` on all new files | All new files | [ ] |
| Q-2 | SPDX license: `// SPDX-License-Identifier: EPL-2.0` on all new files | All new files | [ ] |
| Q-3 | clang-format: all C/C++ files formatted consistently | All .cpp/.h files | [ ] |
| Q-4 | clang-tidy: no CT-P1 violations in production paths | All production .cpp files | [ ] |
| Q-5 | No `#pragma once` missing on header files | All .h files | [ ] |
| Q-6 | Consistent namespace usage (`bios::`, `::can::`, `::systems::`) | All production files | [ ] |
| Q-7 | No magic numbers without comments in register configuration | CAN, clock, UART drivers | [ ] |
| Q-8 | Include guards: `#pragma once` (matches project style) | All headers | [ ] |
| Q-9 | No compiler warnings with `-Wall -Wextra -Wpedantic` | Full build | [ ] |
| Q-10 | No `volatile` misuse (only on hardware register accesses) | All BSP drivers | [ ] |
| Q-11 | Consistent use of `uint32_t` over `unsigned int` for register values | All BSP drivers | [ ] |
| Q-12 | No undefined behavior: signed overflow, null deref, out-of-bounds | All production code | [ ] |

---

## 5. Task Assignments

### Role Definitions

| Role | Abbreviation | Responsibility |
|------|-------------|----------------|
| Build Engineer | **BE** | CMake, presets, toolchain, compilation |
| BSP Developer (CAN) | **BSP-CAN** | BxCanDevice, FdCanDevice, transceivers |
| BSP Developer (Core) | **BSP-CORE** | MCU, clock, timer, UART, interrupts |
| Safety Engineer | **SE** | Hard fault handler, SafetyManager, RTOS hooks |
| Test Engineer | **TE** | Unit test execution, coverage, mock verification |
| Documentation Lead | **DOC** | RST files, Sphinx build, link validation |
| Hardware Validator | **HW** | On-target testing with NUCLEO boards |
| Code Reviewer | **CR** | Code quality, style, security review |

### Assignment Matrix

| Area | Primary | Secondary | Estimated Effort |
|------|---------|-----------|-----------------|
| 4.1 Build System (B-1..B-14) | BE | CR | 4 hours |
| 4.2 MCU Core (M-1..M-11) | BSP-CORE | CR | 2 hours |
| 4.3 Clock Config (C-1..C-10) | BSP-CORE | HW | 4 hours |
| 4.4 Linker/Startup (L-1..L-17) | BSP-CORE | BE | 3 hours |
| 4.5 Interrupts (I-1..I-7) | BSP-CORE | SE | 2 hours |
| 4.6 UART (U-1..U-8) | BSP-CORE | HW | 2 hours |
| 4.7 CAN Devices (CAN-B1..B15, CAN-F1..F17) | BSP-CAN | HW | 8 hours |
| 4.8 CAN Transceivers (T-1..T-14) | BSP-CAN | TE | 4 hours |
| 4.9 System Timer (ST-1..ST-9) | BSP-CORE | HW | 2 hours |
| 4.10 RTOS (R-1..R-12) | SE | BSP-CORE | 4 hours |
| 4.11 Reference App (A-1..A-15) | BSP-CAN | HW | 6 hours |
| 4.12 Safety (S-1..S-9) | SE | CR | 3 hours |
| 4.13 Unit Tests (UT-1..UT-15) | TE | BSP-CAN | 4 hours |
| 4.14 Documentation (D-1..D-11) | DOC | CR | 3 hours |
| 4.15 Code Quality (Q-1..Q-12) | CR | ALL | 4 hours |
| **TOTAL** | | | **~55 hours** |

---

## 6. Test Matrix

### 6.1 Host-Based Tests (No Hardware Required)

| Test | Command | Expected |
|------|---------|----------|
| Configure STM32 unit tests | `cmake --preset tests-stm32-debug` | Success |
| Build STM32 unit tests | `cmake --build --preset tests-stm32-debug` | 0 errors, 0 warnings |
| Run STM32 unit tests | `ctest --preset tests-stm32-debug` | 152 tests pass (per commit message) |
| Run with AddressSanitizer | Rebuild with `-fsanitize=address` | 0 leaks, 0 errors |
| clang-format check | `clang-format --dry-run --Werror` on all new files | 0 violations |
| clang-tidy check | `clang-tidy` on production files | 0 CT-P1 violations |
| Sphinx documentation build | `make -C doc html` | 0 warnings |

### 6.2 Cross-Compilation Tests (ARM Toolchain Required, No Hardware)

| Test | Command | Expected |
|------|---------|----------|
| Configure F413ZH FreeRTOS | `cmake --preset nucleo-f413zh-freertos-gcc` | Success |
| Build F413ZH FreeRTOS | `cmake --build --preset nucleo-f413zh-freertos-gcc` | ELF binary produced |
| Configure G474RE FreeRTOS | `cmake --preset nucleo-g474re-freertos-gcc` | Success |
| Build G474RE FreeRTOS | `cmake --build --preset nucleo-g474re-freertos-gcc` | ELF binary produced |
| F413ZH binary size | `arm-none-eabi-size <elf>` | Flash < 1.5 MB, RAM < 320 KB |
| G474RE binary size | `arm-none-eabi-size <elf>` | Flash < 512 KB, RAM < 128 KB |
| F413ZH map file check | Inspect `.map` | All sections within memory bounds |
| G474RE map file check | Inspect `.map` | All sections within memory bounds |

### 6.3 On-Target Tests (Hardware Required)

| Test | Board | Method | Expected |
|------|-------|--------|----------|
| UART console output | Both | Flash, connect terminal at 115200 | Reset cause printed, lifecycle boot messages |
| LED blink on HardFault | Both | Trigger HardFault | LED blinks, UART shows register dump |
| CAN loopback (bxCAN) | F413ZH | Wire CAN_TX to CAN_RX via transceiver | TX frames received back |
| CAN loopback (FDCAN) | G474RE | Wire CAN_TX to CAN_RX via transceiver | TX frames received back |
| CAN bus-off recovery | Both | Short CAN_H to CAN_L, release | Bus-off detected, auto-recovery |
| IWDG feeding | Both | Run > 30s | No watchdog reset |
| IWDG timeout | Both | Disable tick hook | Watchdog reset within timeout |
| FreeRTOS stack overflow | Both | Allocate large local array in task | Stack overflow hook triggered |
| ThreadX build & boot | Both | Build with THREADX preset, flash | Boot to lifecycle, CAN operational |
| DWT timer accuracy | Both | Measure 1-second interval via UART | Within 1% of wall clock |

---

## 7. Risk Register

| ID | Risk | Impact | Likelihood | Mitigation |
|----|------|--------|------------|------------|
| R1 | CAN bit timing mismatch at different APB1 clocks | CAN bus errors, no communication | Medium | Verify timing with oscilloscope; calculate from reference manual formulas |
| R2 | FDCAN message RAM offset incorrect for FDCAN2/3 | Multi-CAN systems fail | Low | Only FDCAN1 used; offsets documented in header |
| R3 | Hard fault handler dump address conflicts with application data | Corrupted diagnostic data | Low | Linker script should reserve .noinit region |
| R4 | G4 170 MHz boost mode requires specific PWR/RCC sequence | Clock fails to reach target | Medium | Compare with STM32CubeMX generated code |
| R5 | FreeRTOS BASEPRI value masks CAN interrupts | CAN frames lost | High | Verify `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` vs CAN IRQ priority |
| R6 | Startup assembly: FPU not enabled before FP operations | Hard fault on first FP use | Medium | Verify FPU CPACR write in SystemInit or startup |
| R7 | ThreadX port assembly: stale register state after context switch | Task corruption, crashes | Low | Use upstream ThreadX port unchanged |
| R8 | No `.noinit` section in linker script for hard fault dump region | Dump region not reserved | Medium | Add .noinit section or verify no overlap |
| R9 | Test count mismatch: commit says 152, doc says 46 | Confusion in review | Low | Clarify: 152 = all test cases, 46 = transceiver + UART test executables |
| R10 | IWDG timeout value not explicitly configured | Unknown watchdog window | Medium | Verify IWDG prescaler and reload in SystemInit or osHooks |

---

## 8. Sign-off Criteria

### 8.1 Mandatory (Must Pass for Merge)

- [ ] All host-based unit tests pass (`ctest --preset tests-stm32-debug` = 0 failures)
- [ ] Cross-compilation succeeds for both boards with FreeRTOS
- [ ] Binary sizes within flash/RAM limits
- [ ] No clang-tidy CT-P1 violations in production code
- [ ] All copyright and SPDX headers present
- [ ] Sphinx documentation builds without errors
- [ ] Code review approved by at least one BSP developer and one safety engineer
- [ ] CAN bit timing calculations verified against reference manual formulas

### 8.2 Recommended (Should Pass Before Hardware Deployment)

- [ ] UART console output verified on both boards
- [ ] CAN loopback test passed on both boards
- [ ] HardFault register dump verified on one board
- [ ] IWDG feeding verified (system runs > 60s without reset)
- [ ] FreeRTOS and ThreadX variants both boot successfully
- [ ] AddressSanitizer clean on all unit tests
- [ ] DWT timer accuracy within 1% of wall clock

### 8.3 Nice-to-Have (Post-Merge Follow-Up)

- [ ] CAN bus-off recovery test
- [ ] Long-running soak test (24h continuous CAN traffic)
- [ ] Power consumption measurement in idle
- [ ] Flash wear analysis for boot counter
- [ ] Multi-CAN instance test (if FDCAN2/3 needed later)

---

## Appendix A: File Inventory

Total new files: **197**

| Category | Count | Location |
|----------|-------|----------|
| Platform BSP (drivers, headers, tests) | 89 | `platforms/stm32/` |
| 3rd-party (CMSIS, ST headers, RTOS ports) | 36 | `platforms/stm32/3rdparty/`, `bspMcu/include/3rdparty/` |
| Reference app (F413ZH) | 34 | `executables/referenceApp/platforms/nucleo_f413zh/` |
| Reference app (G474RE) | 33 | `executables/referenceApp/platforms/nucleo_g474re/` |
| Documentation (RST) | 27 | `doc/`, board-level `doc/` |
| Build system (CMake) | 5 | Root `CMakeLists.txt`, `CMakePresets.json`, chip cmakes |

## Appendix B: S32K Feature Parity Gap Analysis

Features present in S32K but intentionally absent from STM32:

| S32K Feature | STM32 Status | Reason |
|-------------|-------------|--------|
| ADC driver (bspAdc) | Not ported | NUCLEO boards don't need ADC for reference app |
| Ethernet (bspEthernet) | Not ported | STM32F413/G474 lack ENET peripheral |
| EEPROM driver (bspEepromDriver) | Not ported | No FlexNVM equivalent |
| FTM/PWM drivers | Not ported | Not needed for CAN reference app |
| IO driver (bspIo) | Not ported | GPIO handled directly in main.cpp |
| Cache driver (bspCore) | Not ported | STM32F4/G4 have no instruction/data cache |
| Watchdog driver (safeBspMcuWatchdog) | IWDG used directly | Simpler IWDG vs S32K WDOG |
| Memory protection (safeMemory) | Not ported | Future enhancement |
| BspSystem lifecycle component | Not present | Simplified lifecycle (clock/UART init in SystemInit) |
| Boot header (bootHeader.S) | Not present | STM32 has no flash config field |
| CAN PHY management (CanPhy) | Not present | NUCLEO boards have no external CAN PHY |

## Appendix C: Register Reference Quick-Check

### CAN Bit Timing Verification

**bxCAN (F413ZH):**
- APB1 clock = 48 MHz
- Config: prescaler=6, BS1=13, BS2=2, SJW=1
- f_TQ = 48 MHz / 6 = 8 MHz
- Bit time = 1 + 13 + 2 = 16 TQ
- Baud rate = 8 MHz / 16 = **500 kbps** (correct)
- Sample point = (1 + 13) / 16 = 87.5%

**FDCAN (G474RE):**
- Kernel clock (verify source: PCLK1 or PLL Q)
- Config: prescaler=20, TSEG1=13, TSEG2=1, SJW=0
- Assuming 170 MHz kernel: f_TQ = 170 / 20 = 8.5 MHz
- Bit time = 1 + (13+1) + (1+1) = 17 TQ
- Baud rate = 8.5 MHz / 17 = **500 kbps** (correct)
- Sample point = (1 + 14) / 17 = 88.2%

> **Action item:** Verify FDCAN kernel clock source. If PCLK1 (which may be 170/2 = 85 MHz with APB1 prescaler), the prescaler needs adjustment.

### GPIO Alternate Function Verification

**F413ZH CAN1:**
- PD0 = CAN1_RX (AF9) — verify in DS11581 Table 12
- PD1 = CAN1_TX (AF9) — verify in DS11581 Table 12

**G474RE FDCAN1:**
- PA11 = FDCAN1_RX (AF9) — verify in DS12288 Table 13
- PA12 = FDCAN1_TX (AF9) — verify in DS12288 Table 13

**F413ZH USART3:**
- PD8 = USART3_TX (AF7) — verify in DS11581 Table 12

**G474RE USART2:**
- PA2 = USART2_TX (AF7) — verify in DS12288 Table 13
