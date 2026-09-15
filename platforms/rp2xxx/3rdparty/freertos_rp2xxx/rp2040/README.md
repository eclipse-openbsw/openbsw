# FreeRTOS Custom Port Modification (RP2040)

This directory contains the Raspberry Pi Pico SDK FreeRTOS port for the RP2040.
One of the customizations involves the addition of an application-specific
interrupt service routine (ISR) setup function, `setupApplicationsIsr`.

## Custom Function: `setupApplicationsIsr`

Please refer to ``ManualChanges.diff`` in this directory for the manual
modification. Always remember to reapply this modification whenever you update
the FreeRTOS source code in your project.
