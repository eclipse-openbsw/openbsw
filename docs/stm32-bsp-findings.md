# STM32 BSP Findings: Comparison Against S32K Reference

**Date:** 2026-03-27
**Branch:** `stm32-platform-port`
**Scope:** `platforms/stm32/bsp/` vs `platforms/s32k1xx/bsp/`

---

## 1. Module Coverage

### STM32 BSP modules present

| Module | Description |
|--------|-------------|
| `bspCan` | Low-level CAN device drivers (`BxCanDevice` for F4, `FdCanDevice` for G4) |
| `bspClock` | PLL/RCC configuration (`clockConfig_f4.cpp`, `clockConfig_g4.cpp`) |
| `bspInterruptsImpl` | PRIMASK-based interrupt enable/disable; FreeRTOS and ThreadX variants |
| `bspMcu` | Device headers (CMSIS + ST vendor), startup assembly, linker scripts, soft-reset |
| `bspTimer` | SysTick-based `SystemTimer` |
| `bspUart` | USART driver + include test |
| `bxCanTransceiver` | OpenBSW CAN transceiver for bxCAN (STM32F4) |
| `fdCanTransceiver` | OpenBSW CAN transceiver for FDCAN (STM32G4) |

### S32K reference modules with no STM32 equivalent

| S32K module | Notes |
|-------------|-------|
| `bspAdc` | ADC driver — not yet ported |
| `bspCore` | Cache (LMEM) enable/disable — STM32 has separate I/D caches in FLASH_ACR |
| `bspEepromDriver` | EEPROM via FTFC FlexRAM — no STM32 equivalent |
| `bspEthernet` | ENET/PHY driver — no STM32 Ethernet peripheral on NUCLEO targets |
| `bspFtm` / `bspFtmPwm` | FlexTimer PWM — not yet ported |
| `bspIo` | GPIO abstraction — not yet ported |
| `bspTja1101` | TJA1101 Ethernet PHY — not applicable |

---

## 2. Clock Configuration

### Source files

| Platform | File | Entry point |
|----------|------|-------------|
| STM32F413ZH | `bspClock/src/clockConfig_f4.cpp` | `extern "C" void configurePll()` |
| STM32G474RE | `bspClock/src/clockConfig_g4.cpp` | `extern "C" void configurePll()` |
| S32K148 | `bspClock/src/clockConfig.cpp` | `extern "C" void configurPll()` |

### Clock parameters

| MCU | Source | PLL Config | SYSCLK | APB1 | APB2 | Flash WS |
|-----|--------|-----------|--------|------|------|----------|
| STM32F413ZH | HSE 8 MHz bypass (ST-LINK MCO) | M=8, N=384, P=4 | **96 MHz** | 48 MHz | 96 MHz | 3 WS |
| STM32G474RE | HSI 16 MHz (internal) | M=4, N=85, R=2 | **170 MHz** (boost) | 170 MHz | 170 MHz | 4 WS |
| S32K148 | SOSC/FIRC/SIRC/SPLL (SCG) | Board-specific defines | **80 MHz** | via PCC | via PCC | — |

### Findings

**FINDING-CLK-01 (Naming inconsistency — low severity)**
S32K uses `configurPll()` (missing `e`); STM32 uses `configurePll()` (correct). Both are
platform-specific so no runtime impact, but the inconsistency creates confusion when
reading cross-platform code.

**FINDING-CLK-02 (Integration gap — high severity)**
`configurePll()` is documented as "called from startup assembly before `main()`" (see
`bspClock/doc/index.rst` and `bspMcu/doc/index.rst`), but the startup files
(`startup_stm32f413xx.s` line 63, `startup_stm32g474xx.s` line 63) call `bl SystemInit`,
not `bl configurePll`. `SystemInit` is weakly aliased to `Default_Handler` (infinite loop)
in both startup files. There is **no strong `SystemInit` definition** anywhere in the STM32
BSP that calls `configurePll()`.

- **Effect:** If the application/integration layer does not provide a `SystemInit()` that
  calls `configurePll()`, the board will spin in `Default_Handler` at the first instruction
  of `Reset_Handler` (after copying `.data` and zeroing `.bss`), never reaching `main()`.
- **Required action:** Either provide a `SystemInit()` wrapper that calls `configurePll()`,
  or rename `configurePll` to `SystemInit` in the integration layer. Update the startup
  assembly comment at line 62 to reflect the actual call site.

**FINDING-CLK-03 (Flash latency verification inconsistency — low severity)**
`clockConfig_g4.cpp` verifies the flash latency readback before switching the clock
(RM0440 §3.3.3):
```cpp
while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS) {}
```
`clockConfig_f4.cpp` does **not** include this readback verification after writing
`FLASH->ACR`. The RM0430 §3.3.3 recommends the same check. This is low risk (HSI
startup frequency is well within 0-WS range), but the verification loop should be added
for consistency and defensive correctness.

**FINDING-CLK-04 (G4 HSI dependency — informational)**
The G474 implementation deliberately uses HSI (internal 16 MHz) rather than HSE to
avoid the NUCLEO-G474RE solder-bridge `SB15` routing issue. This is correct and
well-documented in the source file. No action required; note it in board bring-up docs.

---

## 3. CAN Transceiver

### Interface conformance

Both `BxCanTransceiver` and `FdCanTransceiver` correctly extend `::can::AbstractCANTransceiver`
and implement the full `ICanTransceiver` contract (`init`, `open`, `open(frame)`, `close`,
`shutdown`, `mute`, `unmute`, `write(frame)`, `write(frame, listener)`, `getBaudrate`,
`getHwQueueTimeout`).

### Pattern match against S32K `CanFlex2Transceiver`

| Feature | BxCanTransceiver | FdCanTransceiver | CanFlex2Transceiver (S32K) | Match? |
|---------|-----------------|-----------------|---------------------------|--------|
| `TxJobWithCallback` struct | `CANFrame` copy | `CANFrame` copy | `CANFrame const&` reference | STM32 safer |
| `TxQueue` capacity | 3 (`etl::deque`) | 3 (`etl::deque`) | 3 (`etl::deque`) | ✓ |
| Async TX callback | `async::execute` | `async::execute` | `async::execute` | ✓ |
| `cyclicTask()` | ✓ | ✓ | ✓ | ✓ |
| `receiveTask()` | ✓ | ✓ | ✓ | ✓ |
| State machine | CLOSED/INIT/OPEN/MUTED | CLOSED/INIT/OPEN/MUTED | CLOSED/INIT/OPEN/MUTED | ✓ |
| Bus count | 3 | 3 | 8 | NUCLEO has 3 CAN |
| `CanPhy` dependency | absent | absent | required | by design |
| `IEcuPowerStateController` | absent | absent | required | by design |
| `wokenUp()` | absent | absent | present | feature gap |
| `getFirstFrameId()` | absent | absent | present | feature gap |
| `getRxAlive()` | absent | absent | present | feature gap |
| `getOverrunCount()` | absent | absent | present | feature gap |
| RX ISR filter | `nullptr` (accept-all) | `nullptr` (accept-all) | `_filter.getRawBitField()` | acceptable |
| `etl::uncopyable` | absent | absent | present | minor |

### Findings

**FINDING-CAN-01 (TxQueue not cleared on `close()` in FdCanTransceiver — medium severity)**
`FdCanTransceiver::close()` (`FdCanTransceiver.cpp:86-96`) does not call `fTxQueue.clear()`.
`BxCanTransceiver::close()` (`BxCanTransceiver.cpp:90-101`) does. A pending
`TxJobWithCallback` holding a listener reference can remain in the queue after close,
leading to a use-after-free if `canFrameSentAsyncCallback()` fires after the listener is
destroyed.

```cpp
// FdCanTransceiver.cpp:86 — MISSING fTxQueue.clear()
ErrorCode FdCanTransceiver::close() {
    if (getState() == State::CLOSED) return ErrorCode::CAN_ERR_OK;
    fDevice.stop();
    // fTxQueue.clear();  <-- absent, present in BxCanTransceiver
    setState(State::CLOSED);
    return ErrorCode::CAN_ERR_OK;
}
```

**FINDING-CAN-02 (TxQueue not cleared on `mute()` in FdCanTransceiver — medium severity)**
`FdCanTransceiver::mute()` (`FdCanTransceiver.cpp:100-109`) does not call
`fTxQueue.clear()`. `BxCanTransceiver::mute()` (`BxCanTransceiver.cpp:105-116`) does.
Queued frames referencing stale listener objects remain after mute, potentially
causing undefined behaviour when the deferred async callback fires.

**FINDING-CAN-03 (Invalid `State::INITIALIZED` check in `canFrameSentAsyncCallback` — medium severity)**
`FdCanTransceiver::canFrameSentAsyncCallback()` (`FdCanTransceiver.cpp:220-226`) permits
`sendAgain` when the state is `INITIALIZED`:

```cpp
// FdCanTransceiver.cpp:220
if ((State::OPEN == state) || (State::INITIALIZED == state))
    sendAgain = true;
```

`BxCanTransceiver::canFrameSentAsyncCallback()` correctly checks only `State::OPEN`:

```cpp
// BxCanTransceiver.cpp:276
if (getState() == State::OPEN)
    sendAgain = true;
```

Calling `fDevice.transmit()` from `INITIALIZED` state (peripheral not started) is invalid
and will attempt to write to an uninitialised hardware FIFO.

**FINDING-CAN-04 (S32K stores frame reference, STM32 stores copy — informational, STM32 safer)**
`CanFlex2Transceiver::TxJobWithCallback._frame` is `can::CANFrame const&` (reference).
Both STM32 transceivers store `::can::CANFrame` (value copy). The STM32 approach avoids
the dangling-reference risk that arises if the caller's `CANFrame` goes out of scope
before the deferred callback fires. No action required on STM32; consider fixing S32K.

**FINDING-CAN-05 (Missing extended transceiver APIs — low severity)**
`wokenUp()`, `getFirstFrameId()`, `resetFirstFrame()`, `getRxAlive()`, `getOverrunCount()`
from `CanFlex2Transceiver` are absent from both STM32 transceivers. If application code
calls these via a base-class pointer it will fail to compile. Add stubs or implement
where needed.

---

## 4. Linker Scripts

### Memory maps

| Script | FLASH origin | FLASH size | RAM origin | RAM size |
|--------|-------------|-----------|-----------|---------|
| `STM32F413ZHxx_FLASH.ld` | `0x08000000` | **1536 KB** | `0x20000000` | **320 KB** |
| `STM32G474RExx_FLASH.ld` | `0x08000000` | **512 KB** | `0x20000000` | **128 KB** |

Both values are correct per the respective datasheets:
- STM32F413ZH: 1.5 MB Flash, 256 KB SRAM1 + 64 KB SRAM2 = 320 KB contiguous
- STM32G474RE: 512 KB Flash, 96 KB SRAM1 + 32 KB SRAM2 = 128 KB contiguous at `0x20000000`

### Section layout (both scripts identical except memory sizes)

| Section | Region | Notes |
|---------|--------|-------|
| `.isr_vector` | FLASH | `KEEP` — not garbage-collected |
| `.text` | FLASH | Includes `.glue_7`, `.glue_7t`, `.eh_frame`, `.init`, `.fini` |
| `.rodata` | FLASH | |
| `.ARM.extab` / `.ARM` | FLASH | C++ exception unwind tables |
| `.preinit_array`, `.init_array`, `.fini_array` | FLASH | C++ ctors/dtors |
| `.data` | RAM (LMA: FLASH) | Includes `.RamFunc*` for RAM-executed code |
| `.bss` | RAM | |
| `._user_heap_stack` | RAM | Min heap 512 B, min stack 1024 B |
| `/DISCARD/` | — | `libc.a`, `libm.a`, `libgcc.a` discarded |

### S32K comparison

No production linker scripts exist under `platforms/s32k1xx/bsp/`. The only `.ld` file
found is `platforms/s32k1xx/3rdparty/threadx/ports/cortex_m4/gnu/example_build/sample_threadx.ld`
(third-party example). S32K linker scripts are apparently provided at the
application/project level, not in the BSP.

### Findings

**FINDING-LD-01 (Min stack 1024 B may be insufficient — medium severity)**
`_Min_Stack_Size = 0x400` (1024 bytes) is used only to reserve space in
`._user_heap_stack`. In an RTOS environment (FreeRTOS or ThreadX), the main/MSP stack is
used during early startup and interrupt handlers; task stacks are allocated separately
from the heap. 1024 B is borderline for nested interrupt handlers with printf-style
formatting. Consider increasing to 0x1000 (4 KB) or documenting the expected worst-case
stack depth.

**FINDING-LD-02 (CCM SRAM not mapped on G474 — informational)**
STM32G474RE has 32 KB CCM-SRAM at `0x10000000` (separate from the main SRAM bank). This
region is not mapped in `STM32G474RExx_FLASH.ld`. No action required unless performance-
critical data or real-time ISR stacks need placement in CCM. If CCM is used in future,
add a `CCMRAM` region and corresponding sections.

**FINDING-LD-03 (No stack overflow guard — low severity)**
Neither linker script inserts a stack canary or ASSERT to detect heap/stack collision.
Adding `ASSERT((_sstack > _ebss + _Min_Heap_Size), "Stack/heap overlap!")` is defensive
practice for bare-metal linker scripts.

---

## 5. Startup Assembly

### Structure (both F413 and G474 identical flow)

```
Reset_Handler:
  1. ldr sp, =_estack          — set initial stack pointer
  2. copy .data FLASH → RAM    — _sidata / _sdata / _edata
  3. zero .bss                 — _sbss / _ebss
  4. bl SystemInit             — clock / peripheral init
  5. bl __libc_init_array      — C++ static constructors
  6. bl main
```

### Vector counts

| Startup file | External IRQs | Reference |
|-------------|---------------|-----------|
| `startup_stm32f413xx.s` | 102 (IRQn 0–101) | RM0430 Table 40 ✓ |
| `startup_stm32g474xx.s` | 102 (IRQn 0–101) | RM0440 Table 97 ✓ |

### Findings

**FINDING-STARTUP-01 (Cross-reference to FINDING-CLK-02)**
Both startup files declare `SystemInit` as a weak alias to `Default_Handler` (lines 416-417
in F413, lines 430-431 in G474). This is the call site where `configurePll()` must be
reached. See FINDING-CLK-02 for the full description and required action.

**FINDING-STARTUP-02 (`.fpu softvfp` disables FPU in startup — informational)**
Both startup files use `.fpu softvfp`. This is correct for the startup assembly itself
(no floating-point operations in `Reset_Handler`). The Cortex-M4F FPU co-processor access
is enabled via `SCB->CPACR` in `SystemInit` (standard CMSIS practice). No action required.

---

## 6. BxCanDevice vs FdCanDevice vs FlexCANDevice

| Feature | BxCanDevice (F4) | FdCanDevice (G4) | FlexCANDevice (S32K) |
|---------|-----------------|-----------------|---------------------|
| TX hardware | 3 mailboxes | TX FIFO (message RAM) | 16 message buffers |
| RX hardware | 2 FIFOs (FIFO0/1) | 2 FIFOs (FIFO0/1, msg RAM) | Configurable RX buffers |
| HW filter | 28 filter banks (mask/list) | Standard + extended filter elements | Per-buffer ID match |
| SW RX queue | 32 frames circular | 32 frames circular | `etl::queue<CANFrame, 32>` |
| Error counters | `CAN->ESR` (TEC/REC/BOFF) | `FDCAN->ECR` + `FDCAN->PSR` | `FLEXCAN->ECR` |
| CAN-FD capable | No | Yes (used in classic mode) | No |
| `isBusOff()` | ✓ | ✓ | `getBusOffState()` enum |
| `configureFilterList()` | ✓ (ID list mode) | ✓ (std ID elements) | per-buffer mask registers |
| `disableRxInterrupt()` | ✓ | ✓ | separate IMASK approach |

**FINDING-DEV-01 (FdCanDevice has public `enterInitMode` / `leaveInitMode` — low severity)**
`FdCanDevice.h` declares `enterInitMode()`, `leaveInitMode()`, `configureBitTiming()`,
`configureMessageRam()`, `configureGpio()`, and `enablePeripheralClock()` as `public`
members (lines 189-194). In `BxCanDevice.h` these are `private` (lines 213-218). Exposing
these in `FdCanDevice` breaks encapsulation and allows callers to corrupt peripheral state.
Move them to `private` or `protected`.

---

## 7. Summary Table

| Area | Status | Critical Findings |
|------|--------|-------------------|
| Clock config — F413 | Functionally correct | FINDING-CLK-02 (integration gap), CLK-03 (no readback verify) |
| Clock config — G474 | Correct, robust | FINDING-CLK-02 (integration gap) |
| BxCanTransceiver | Matches S32K reference | — |
| FdCanTransceiver | Mostly matches | FINDING-CAN-01, CAN-02, CAN-03 (TxQueue/state bugs) |
| Linker script F413 | Correct memory map | FINDING-LD-01 (small stack), LD-03 (no overflow guard) |
| Linker script G474 | Correct memory map | FINDING-LD-01 (small stack), LD-02 (no CCM), LD-03 |
| Startup F413 | Correct | FINDING-STARTUP-01 (see CLK-02) |
| Startup G474 | Correct | FINDING-STARTUP-01 (see CLK-02) |
| BxCanDevice | Well encapsulated | — |
| FdCanDevice | Functional | FINDING-DEV-01 (public internals) |
| Module coverage | Partial | ADC, GPIO, PWM, Ethernet not yet ported |

---

## 8. Action Items

| Priority | Finding | Action |
|----------|---------|--------|
| **HIGH** | FINDING-CLK-02 | Provide strong `SystemInit()` (in application or integration layer) that calls `configurePll()`. Update startup comment at line 62 of both `.s` files. |
| **MEDIUM** | FINDING-CAN-01 | Add `fTxQueue.clear()` to `FdCanTransceiver::close()`. |
| **MEDIUM** | FINDING-CAN-02 | Add `fTxQueue.clear()` to `FdCanTransceiver::mute()`. |
| **MEDIUM** | FINDING-CAN-03 | Remove `State::INITIALIZED` from `sendAgain` guard in `FdCanTransceiver::canFrameSentAsyncCallback()`. |
| **MEDIUM** | FINDING-LD-01 | Increase `_Min_Stack_Size` to `0x1000` or document the accepted worst-case ISR stack depth. |
| **LOW** | FINDING-CLK-03 | Add FLASH latency readback loop to `clockConfig_f4.cpp` after writing `FLASH->ACR`. |
| **LOW** | FINDING-DEV-01 | Move `enterInitMode`, `leaveInitMode`, `configureBitTiming`, `configureMessageRam`, `configureGpio`, `enablePeripheralClock` to `private` in `FdCanDevice`. |
| **LOW** | FINDING-LD-03 | Add `ASSERT` for stack/heap collision detection in both linker scripts. |
| **INFO** | FINDING-CAN-04 | No action on STM32 side; consider fixing S32K `CanFlex2Transceiver` to store frame copy. |
| **INFO** | FINDING-CAN-05 | Implement or stub `wokenUp`, `getFirstFrameId`, `getRxAlive`, `getOverrunCount` if needed by application. |
| **INFO** | FINDING-LD-02 | Map CCM-SRAM region in G474 linker script if latency-critical data placement is needed. |
