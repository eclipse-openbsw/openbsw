User documentation
==================

FTM Operation Modes
-------------------

There are 7 FTM modes:

- Edge-align PWM(EPWM) mode
- Center-align PWM(CPWM) mode
- Complimentary mode and dead-time insertion
- Combine mode
- Single-edge capture mode
- Dual-edge capture mode
- Quadrature decoder mode

Edge-align PWM (EPWM) mode
++++++++++++++++++++++++++

 - In the edge-align PWM mode, the FTM counter counts up from the FTM_CNTIN value to the
   FTM_MOD value. All FTM channels’ signals align at the edge when the FTM counter changes from the
   MOD value to the CNTIN value.

 - The edge-align PWM mode is selected when QUADEN = 0, DECAPEN = 0, MCOMBINE = 0,
   COMBINE = 0, CPWMS = 0 and MSnB = 1.

 - The edge-align PWM period is determined by MOD – CNTIN + 0x0001 and the pulse width (or the
   duty cycle) is determined by CnV – CNTIN or MOD – CNTIN – CnV, depending on the control bits
   ELSnB:ELSnA.