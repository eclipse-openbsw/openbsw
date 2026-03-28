# STM32 Application Layer Findings

**Branch:** `stm32-platform-port`
**Reviewed:** 2026-03-27
**Scope:** Platform `main.cpp` entry points, referenceApp lifecycle/OS hooks, FreeRTOS configuration, upstream code impact.

---

## 1. Platform `main.cpp` — NUCLEO-F413ZH

**File:** `executables/referenceApp/platforms/nucleo_f413zh/main/src/main.cpp`

### Boot sequence

```
main()
  ├─ diag_puts("[diag] main() enter")          // raw USART3 before any driver
  ├─ IWDG->KR = 0xAAAAU                        // pet watchdog early
  ├─ safeSupervisorConstructor.construct()
  ├─ platform::staticBsp.init()                // initSystemTimer + UART init
  ├─ app_main()                                // → etl assert setup → app::run()
  └─ return 1  (unreachable under normal operation)
```

### `SystemInit()` (runs before `main()`, called from startup assembly)

- Calls `configurePll()` to bring core to 96 MHz.
- Enables `GPIOB` and drives `PB0` (LD1, green LED) high immediately.
- Configures `PD8` as `AF7` (USART3_TX) and brings up USART3 at 115 200 Bd
  (`BRR = 417` → 48 MHz APB1 / 115 200 ≈ 416.67 — correctly rounded).
- Reads `RCC->CSR`, prints a human-readable reset-cause string (`LPWR`, `WWDG`,
  `IWDG`, `SFT`, `POR`, `PIN`, `BOR`) plus the raw CSR hex over USART3.
- Clears reset flags via `RCC_CSR_RMVF`.

### `HardFault_Handler()`

- Immediately pets IWDG (`0xAAAA`) so the watchdog does not fire during
  diagnostic output.
- Reads faulting stack pointer (MSP or PSP, determined by LR bit 2).
- Emits `PC`, `LR`, `CFSR`, `HFSR`, and `SP` over USART3 in hex.
- Enters an infinite blink-and-IWDG-feed loop (`PB0` toggles).

### `platformLifecycleAdd()`

```cpp
// Guarded by PLATFORM_SUPPORT_CAN
if (level == 2U)
    lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
```

- CAN is optional; `CanSystem` is registered at lifecycle level 2 only when
  `PLATFORM_SUPPORT_CAN` is defined at compile time.
- `getCanSystem()` (in `namespace systems`) returns the same
  `etl::typed_storage`-managed instance.

### `putchar_()` / `diag_puts()`

Both route characters directly through `USART3->DR` with polling, providing
a pre-scheduler debug channel used by `etl::print`.

---

## 2. Platform `main.cpp` — NUCLEO-G474RE

**File:** `executables/referenceApp/platforms/nucleo_g474re/main/src/main.cpp`

### Boot sequence

```
main()
  ├─ safeSupervisorConstructor.construct()
  ├─ platform::staticBsp.init()
  ├─ app_main()
  └─ return 1
```

**Difference from F413ZH:** No `diag_puts` calls in `main()` itself (no
pre-`app_main` banner). The G474RE board is expected to use a debugger probe
or rely on `SystemInit()` UART output.

### `SystemInit()`

- `configurePll()` sets 170 MHz.
- Drives `PA5` (LD2) high.
- Brings up USART2 on `PA2 AF7` at 115 200 Bd
  (`BRR = 1476` → assumes PCLK1 = SYSCLK = 170 MHz).
- Does **not** print a reset-cause string (unlike F413ZH).
- Waits for `USART_ISR_TEACK` (TE acknowledge) instead of `USART_SR_TC`
  — correct for the G4 USART register layout.

### `HardFault_Handler()`

- Does **not** feed IWDG (the G474RE variant does not pet the watchdog in fault
  context).
- Dumps `PC`, `LR`, `CFSR` (reads SCB->CFSR via raw address `0xE000ED28`).
- Does **not** dump `HFSR` or `SP` (less information than the F413ZH handler).
- Blinks `PA5` indefinitely; no watchdog feed in the loop.

### `platformLifecycleAdd()`

```cpp
// No PLATFORM_SUPPORT_CAN guard — CAN is mandatory on G474RE
if (level == 2U)
    lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
```

### Notable differences from F413ZH

| Aspect | NUCLEO-F413ZH | NUCLEO-G474RE |
|--------|--------------|--------------|
| Debug UART | USART3 (`PD8`, SR/DR registers) | USART2 (`PA2`, ISR/TDR registers) |
| Reset-cause print in `SystemInit` | Yes | No |
| `diag_puts` in `main()` | Yes (4 checkpoints) | No |
| `HardFault` IWDG feed | Yes (entry + loop) | No |
| `HardFault` HFSR dump | Yes | No |
| CAN guard macro | `#ifdef PLATFORM_SUPPORT_CAN` | Unconditional |
| `putchar_` defined | Yes (USART3) | No |

---

## 3. Application `main.cpp` (Shared)

**File:** `executables/referenceApp/application/src/main.cpp`

- Platform-independent; called as `app_main()` from each platform entry point.
- Registers `etl_assert_function` for ETL assertion handling.
  - On POSIX: calls `backtrace_symbols_fd` + prints file/line + `std::abort()`.
  - On embedded targets: calls `std::abort()` directly.
- Delegates to `::app::run()`.
- Contains no platform-specific code. **No changes** from upstream.

---

## 4. referenceApp Lifecycle Hooks

### `StaticBsp::init()` (both platforms, identical)

**Files:**
- `executables/referenceApp/platforms/nucleo_f413zh/main/src/lifecycle/StaticBsp.cpp`
- `executables/referenceApp/platforms/nucleo_g474re/main/src/lifecycle/StaticBsp.cpp`

```cpp
void StaticBsp::init() {
    initSystemTimer();
    bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL).init();
}
```

Initialises the system tick timer and the BSP UART driver (TERMINAL instance).
Called once from `main()` before `app_main()`.

### `CanSystem` lifecycle component

**F413ZH header:** `executables/referenceApp/platforms/nucleo_f413zh/main/include/systems/CanSystem.h`

Inherits from:
- `lifecycle::SingleContextLifecycleComponent` — participates in lifecycle transitions.
- `can::ICanSystem` — provides `getCanTransceiver()`.
- `etl::singleton_base<CanSystem>` — ISR trampolines access the instance without
  a global pointer.

Lifecycle contract:
- `init()` → calls `transitionDone()` immediately (no hardware setup yet).
- `run()` → enables `CanRxRunnable`, calls `transceiver.init()` + `transceiver.open()`,
  configures NVIC, enables CAN IRQs, calls `transitionDone()`.
- `shutdown()` → closes/shuts down transceiver, disables `CanRxRunnable`,
  calls `transitionDone()`.

---

## 5. FreeRTOS OS Hooks

### NUCLEO-F413ZH (`osHooks/freertos/osHooks.cpp`)

| Hook | Behaviour |
|------|-----------|
| `vApplicationStackOverflowHook` | `while(true){}` — fail-closed safe state |
| `vApplicationMallocFailedHook` | `while(true){}` — fail-closed safe state |
| `vApplicationTickHook` | `IWDG->KR = 0xAAAAU` — feeds IWDG every tick (~1 ms) |

**Finding:** `configUSE_MALLOC_FAILED_HOOK` is set to `0` in `FreeRtosPlatformConfig.h`,
yet `vApplicationMallocFailedHook` is defined. FreeRTOS will never call this hook;
the function body is dead code. This is harmless (no linker error) but inconsistent.
The hook should either be removed or `configUSE_MALLOC_FAILED_HOOK` set to `1`.

### NUCLEO-G474RE (`osHooks/freertos/osHooks.cpp`)

| Hook | Behaviour |
|------|-----------|
| `vApplicationStackOverflowHook` | `while(true){}` |
| `vApplicationMallocFailedHook` | `while(true){}` |
| `vApplicationTickHook` | **Not implemented** |

No `configUSE_TICK_HOOK` override in the G474RE platform config, so the base
value of `0` applies. The G474RE does not feed IWDG from the tick hook. This is
consistent with the assumption that no previous firmware on G474RE has started
the IWDG, but should be verified on-target.

### ThreadX hooks — NUCLEO-F413ZH (`osHooks/threadx/osHooks.cpp`)

| Hook | Behaviour |
|------|-----------|
| `_tx_execution_thread_enter` | Calls `asyncEnterTask()` — integrates with async profiling |
| `_tx_execution_thread_exit` | Calls `asyncLeaveTask()` |
| `tx_low_power_enter` | `IWDG->KR = 0xAAAAU` before low-power entry |
| `tx_low_power_exit` | No-op |
| `vIllegalISR` | `printf` + `for(;;)` |

---

## 6. FreeRTOS Configuration

### Base config: `libs/bsw/asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h`

**Upstream file — not modified by STM32 port.**

Key settings:
- `configSUPPORT_STATIC_ALLOCATION = 1`, `configSUPPORT_DYNAMIC_ALLOCATION = 0`
  — static allocation only; no heap allocator used.
- `configUSE_PREEMPTION = 1`.
- `configTICK_RATE_HZ = 1000000 / ASYNC_CONFIG_TICK_IN_US` — tick rate derived
  from async framework configuration.
- `configUSE_IDLE_HOOK = 1`, `configUSE_TICK_HOOK = 0` (overridable per platform).
- Trace macros `traceTASK_CREATE`, `traceTASK_SWITCHED_IN`, `traceTASK_SWITCHED_OUT`
  hook into `asyncEnterTask` / `asyncLeaveTask` for run-time statistics.
- Includes `os/FreeRtosPlatformConfig.h` for device-specific overrides.

### NUCLEO-F413ZH override: `freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`

| Parameter | Value | Notes |
|-----------|-------|-------|
| `configCPU_CLOCK_HZ` | 96 000 000 | Matches `configurePll()` output |
| `configMINIMAL_STACK_SIZE` | 512 words | Overrides base `0` |
| `configTOTAL_HEAP_SIZE` | 512 bytes | Minimal — static alloc only |
| `configAPPLICATION_ALLOCATED_HEAP` | 1 | Heap buffer provided by application |
| `configCHECK_FOR_STACK_OVERFLOW` | 2 | Full pattern-check at context switch |
| `configUSE_MALLOC_FAILED_HOOK` | 0 | See finding §5 above |
| `configUSE_TICK_HOOK` | **1** | Enables IWDG feed via `vApplicationTickHook` |
| `configUSE_TICKLESS_IDLE` | 0 | Continuous tick (no tickless) |
| `configPRIO_BITS` | `__NVIC_PRIO_BITS` (4) | 16 priority levels |
| `configLIBRARY_LOWEST_INTERRUPT_PRIORITY` | 0x0F | |
| `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` | **0x05** | BASEPRI = 0x50 |
| `configGENERATE_RUN_TIME_STATS` | 1 | Uses `getSystemTimeUs32Bit()` |
| Timer task name | `"TIMER_OS"` | |
| Handler aliases | `SVC_Handler`, `PendSV_Handler`, `SysTick_Handler` | CMSIS naming |

### NUCLEO-G474RE override: `freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`

Nearly identical to F413ZH except:

| Parameter | F413ZH | G474RE | Notes |
|-----------|--------|--------|-------|
| `configCPU_CLOCK_HZ` | 96 MHz | **170 MHz** | |
| `configUSE_TICK_HOOK` | 1 | **not set** (→ 0) | No IWDG feed from tick |
| `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` | 0x05 | **0x06** | FDCAN ISR at priority 5 (0x50) must not be masked; BASEPRI = 0x60 leaves it unmasked |

The G474RE config includes an explanatory comment:

```
// CAN ISR at priority 5 (0x50) must not be masked by BASEPRI.
// BASEPRI = (this value << 4). With 5: BASEPRI=0x50 masks priority 5.
// With 6: BASEPRI=0x60, priority 5 (0x50) is NOT masked.
```

This is a deliberate and well-documented difference from the F413ZH value.

---

## 7. Upstream Code Impact Analysis

### Git status

```
?? docs/        (untracked — this report directory only)
```

No upstream tracked files have uncommitted modifications. The working tree is clean.

### Files modified vs. `main` branch

The STM32 port (`commit 25e8dee6`) touches two pre-existing root files:

| File | Nature of change |
|------|-----------------|
| `CMakeLists.txt` | Added `NUCLEO_F413ZH` / `NUCLEO_G474RE` cases to platform and Rust target blocks (+17 lines, additive) |
| `CMakePresets.json` | Added two new configure presets for the STM32 targets (+79 lines, additive) |

**All other STM32 additions are new files.** No existing library, BSP, or referenceApp source was modified by the STM32 port commit.

Earlier commits on this branch (`49933365`, `c2e895c2`, `fdf7872d`, `4246a530`)
modified upstream files for unrelated reasons (clang-tidy CT-P1 fixes, logger
vararg fixes, middleware additions, Ethernet note). Those changes are independent
of the STM32 port and do not affect platform correctness.

---

## 8. Findings Summary

| # | Severity | Location | Finding |
|---|----------|----------|---------|
| F-01 | **Low** | `nucleo_f413zh/freeRtosCoreConfiguration/…/FreeRtosPlatformConfig.h:33` | `configUSE_MALLOC_FAILED_HOOK = 0` but `vApplicationMallocFailedHook` is defined in `osHooks.cpp`. Hook is dead code; either enable the config flag or remove the function. |
| F-02 | **Info** | `nucleo_g474re/main/src/main.cpp:132-138` | No `diag_puts` checkpoints in `main()` on G474RE. Pre-`app_main` failures are invisible unless a debugger is attached. Consider adding minimal USART2 banners for parity with F413ZH. |
| F-03 | **Info** | `nucleo_g474re/main/src/main.cpp:51-101` | G474RE `HardFault_Handler` does not dump `HFSR` or `SP`, and does not feed the IWDG in the fault loop. F413ZH handler provides richer diagnostics. Recommend alignment for consistency. |
| F-04 | **Info** | `nucleo_g474re/freeRtosCoreConfiguration/…/FreeRtosPlatformConfig.h` | No `configUSE_TICK_HOOK` override (remains `0`). IWDG is not fed from the FreeRTOS tick on G474RE. Acceptable if no prior firmware has started the IWDG, but should be verified on hardware. |
| F-05 | **OK** | `CMakeLists.txt`, `CMakePresets.json` | Two upstream root files modified — both changes are purely additive (new `elseif` blocks and new preset entries). No existing functionality altered. |
| F-06 | **OK** | All `platforms/stm32/` and `nucleo_*/` directories | All STM32 platform files are new additions. Zero existing library or BSP source files were modified by the platform port. |
| F-07 | **OK** | `nucleo_g474re/freeRtosCoreConfiguration/…/FreeRtosPlatformConfig.h:86` | `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 0x06` (vs `0x05` on F413ZH) is intentional and correctly documented in the file to allow FDCAN ISR at NVIC priority 5 to remain unmasked. |

---

## 9. File Index

| Role | F413ZH path | G474RE path |
|------|------------|------------|
| Platform entry point | `executables/referenceApp/platforms/nucleo_f413zh/main/src/main.cpp` | `executables/referenceApp/platforms/nucleo_g474re/main/src/main.cpp` |
| Application entry | `executables/referenceApp/application/src/main.cpp` | ← same file |
| StaticBsp init | `…/nucleo_f413zh/main/src/lifecycle/StaticBsp.cpp` | `…/nucleo_g474re/main/src/lifecycle/StaticBsp.cpp` |
| FreeRTOS OS hooks | `…/nucleo_f413zh/main/src/osHooks/freertos/osHooks.cpp` | `…/nucleo_g474re/main/src/osHooks/freertos/osHooks.cpp` |
| ThreadX OS hooks | `…/nucleo_f413zh/main/src/osHooks/threadx/osHooks.cpp` | `…/nucleo_g474re/main/src/osHooks/threadx/osHooks.cpp` |
| FreeRTOS platform config | `…/nucleo_f413zh/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` | `…/nucleo_g474re/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h` |
| FreeRTOS base config | `libs/bsw/asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h` | ← same file |
| CanSystem header | `…/nucleo_f413zh/main/include/systems/CanSystem.h` | `…/nucleo_g474re/main/include/systems/CanSystem.h` |
