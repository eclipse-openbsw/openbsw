# Vendored ThreadX CPU ports for the RP2xxx platform

Ports taken from eclipse-threadx/threadx, tag ``v6.4.3.202503_rel``, matching the
kernel vendored in ``libs/3rdparty/threadx``:

| Directory              | Upstream path           | Used for |
|------------------------|-------------------------|----------|
| ``ports/cortex_m0``    | ``ports/cortex_m0``     | RP2040   |
| ``ports/cortex_m33``   | ``ports/cortex_m33``    | RP2350   |

The port is selected by ``RP2XXX_CHIP``.

## Notes on the Cortex-M33 port

The RP2350 is run without TrustZone, so ``TX_SINGLE_MODE_SECURE`` is defined.
That compiles out the secure stack management, and with it the port's
``SVC_Handler``: in single mode ThreadX only uses SVC for secure stack calls,
and the first thread is entered through PendSV.

The port also ships a ``tx_initialize_low_level.S``. It is excluded from the
build because it hardcodes the SysTick reload value, refers to ``_vectors`` and
``__RAM_segment_used_end__`` which this platform does not define, and defines
``HardFault_Handler`` and ``UsageFault_Handler``, which would collide with the
``hardFaultHandler`` module. The boards provide their own implementation under
``main/src/bsp/threadx``, in the same way the Cortex-M4 platforms do.

## Local modifications

See ``ManualChanges.diff``.
