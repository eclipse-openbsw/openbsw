# FreeRTOS Custom Port Modification

This repository contains a custom implementation of FreeRTOS for the S32K1xx platform. One of the customizations involves the addition of an application-specific interrupt service routine (ISR) setup function, `setupApplicationsIsr`.

## Custom Function: `setupApplicationsIsr`

Please refer to ``central/workspaces/referenceApp/platforms/s32k1xx/3rdparty/ManualChanges.diff`` for the making manual modification.
Always remember to reapply this modification whenever you update the FreeRTOS source code in your project.