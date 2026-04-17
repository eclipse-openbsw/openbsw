bspClock
========

Overview
--------

The ``bspClock`` module configures the system clock PLL for STM32F4 and STM32G4
families. Each family has its own translation unit (``clockConfig_f4.cpp`` and
``clockConfig_g4.cpp``); the public API is a single ``extern "C"`` function
``configurePll()`` declared in ``clock/clockConfig.h``.

``configurePll()`` is called from the startup assembly (``Reset_Handler``) before
``main()``. It must not use heap, RTOS primitives, or any BSW services.

STM32F4 (STM32F413ZH) -- 96 MHz
--------------------------------

Clock source
    HSE 8 MHz bypass (ST-LINK MCO on NUCLEO-F413ZH).

PLL parameters
    - ``PLLM = 8`` -- VCO input = 8 / 8 = 1 MHz
    - ``PLLN = 384`` -- VCO output = 1 * 384 = 384 MHz
    - ``PLLP = 4`` (register value ``1``) -- SYSCLK = 384 / 4 = **96 MHz**
    - ``PLLQ`` -- not configured (USB not used)

Bus prescalers
    - AHB = SYSCLK / 1 = 96 MHz (``HPRE`` default)
    - APB1 = SYSCLK / 2 = 48 MHz (max 50 MHz per datasheet)
    - APB2 = SYSCLK / 1 = 96 MHz

Flash latency
    3 wait states at 96 MHz / 3.3 V. Prefetch, instruction cache, and data cache
    are all enabled (``FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN``).

Sequence
    1. Enable HSE bypass, wait ``HSERDY``.
    2. Write ``RCC->PLLCFGR`` with M/N/P values and HSE source.
    3. Enable PLL, wait ``PLLRDY``.
    4. Set flash latency with caches enabled.
    5. Switch SYSCLK to PLL via ``RCC->CFGR``, wait ``SWS == PLL``.
    6. Update ``SystemCoreClock = 96000000``.

STM32G4 (STM32G474RE) -- 170 MHz
---------------------------------

Clock source
    HSI 16 MHz (internal). HSE is not used because the NUCLEO-G474RE solder
    bridge SB15 may not route ST-LINK V3 MCO to PH0/OSC_IN on all board
    revisions.

PLL parameters
    - ``PLLM = 4`` (register value ``M-1 = 3``) -- VCO input = 16 / 4 = 4 MHz
    - ``PLLN = 85`` -- VCO output = 4 * 85 = 340 MHz
    - ``PLLR = 2`` (register value ``0``) -- SYSCLK = 340 / 2 = **170 MHz**
    - ``PLLREN`` enabled to route PLL R output to SYSCLK.

Bus prescalers
    - AHB = SYSCLK / 1 = 170 MHz (after boost transition)
    - APB1 = SYSCLK / 1 = 170 MHz
    - APB2 = SYSCLK / 1 = 170 MHz

Flash latency
    4 wait states at 170 MHz / 3.3 V with boost mode. Prefetch, instruction
    cache, and data cache are enabled. The ``FLASH_ACR`` write uses a
    read-modify-write pattern to preserve ``DBG_SWEN`` (bit 18) -- clearing this
    bit kills SWD debug flash access. A readback loop verifies the latency
    setting before proceeding (RM0440 section 3.3.3).

Boost mode sequence (RM0440 section 6.1.5)
    1. Enable PWR peripheral clock (``RCC->APB1ENR1 |= PWREN``), read-back delay.
    2. Clear ``PWR_CR5_R1MODE`` to enable voltage Range 1 boost mode.
    3. Wait for ``VOSF`` flag to clear (voltage scaling stable).
    4. Configure PLL with HSI source and M/N/R values, enable PLL, wait
       ``PLLRDY``.
    5. Set flash latency 4 WS (read-modify-write preserving ``DBG_SWEN``).
    6. Set AHB prescaler to ``/2`` and switch SYSCLK to PLL in one
       ``RCC->CFGR`` write. Wait ``SWS == PLL``.
    7. Busy-wait >= 1 us for voltage regulator settle (100 iterations at
       85 MHz).
    8. Restore AHB prescaler to ``/1``.
    9. Update ``SystemCoreClock = 170000000``.

Design Notes
------------

- Each STM32 family has its own ``.cpp`` file; only the matching translation
  unit is compiled via the build system. The header ``clockConfig.h`` provides
  the common ``configurePll()`` declaration.
- ``SystemCoreClock`` is initialised to 16 MHz (HSI default) as a file-scope
  variable and updated to the final frequency at the end of ``configurePll()``.
  This global is consumed by CMSIS and FreeRTOS for tick rate calculations.
- No interrupts are enabled at the time ``configurePll()`` runs; all waits are
  polling loops on hardware ready flags.
