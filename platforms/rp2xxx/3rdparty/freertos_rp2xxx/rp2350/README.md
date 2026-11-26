# FreeRTOS Custom Port Modification (RP2350)

The RP2350 is a Cortex-M33. It is run here without TrustZone and without the
MPU, so the generic FreeRTOS ARM Cortex-M4F port is used rather than the
pico-sdk SMP port used for the RP2040. The port sources are the same ones that
``platforms/s32k1xx/3rdparty/freertos_cm4_sysTick`` uses, including the
application-specific interrupt service routine setup function
`setupApplicationsIsr`.

Because the generic Cortex-M port is used, the tick rate is taken from
`configSYSTICK_CLOCK_HZ` (see the board `FreeRtosPlatformConfig.h`) instead of
from `clock_get_hz(clk_sys)`, and the `pico_sync` / `pico_time` interop layers
are disabled.

## Custom Function: `setupApplicationsIsr`

Please refer to ``ManualChanges.diff`` in this directory for the manual
modification. Always remember to reapply this modification whenever you update
the FreeRTOS source code in your project.
