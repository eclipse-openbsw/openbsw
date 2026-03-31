/**
 * \file startup_stm32g474xx.s
 * \brief Startup for STM32G474xx — vector table + Reset_Handler
 *
 * Complete vector table with all 102 interrupt vectors per RM0440.
 * Based on ST CMSIS device support for STM32G474xx.
 *
 * SPDX-License-Identifier: EPL-2.0
 */

  .syntax unified
  .cpu cortex-m4
  .fpu fpv4-sp-d16
  .thumb

.global g_pfnVectors
.global Default_Handler

.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack

/* Copy .data from FLASH to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

/* Zero fill .bss */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Enable FPU: set CP10/CP11 full access (required for ThreadX — FreeRTOS
   enables this in port.c, but ThreadX does not) */
  ldr r0, =0xE000ED88
  ldr r1, [r0]
  orr r1, r1, #(0xF << 20)
  str r1, [r0]
  dsb
  isb

/* Call SystemInit, C++ constructors, and main */
  bl SystemInit
  bl __libc_init_array
  bl main
  bx lr

.size Reset_Handler, .-Reset_Handler

  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler

/******************************************************************************
 * Vector table for STM32G474xx (102 external interrupts, RM0440 Table 97)
 ******************************************************************************/
  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object

g_pfnVectors:
  /* Cortex-M4 system exceptions */
  .word _estack                         /* Top of Stack                        */
  .word Reset_Handler                   /* Reset Handler                       */
  .word NMI_Handler                     /* NMI Handler                         */
  .word HardFault_Handler               /* Hard Fault Handler                  */
  .word MemManage_Handler               /* MPU Fault Handler                   */
  .word BusFault_Handler                /* Bus Fault Handler                   */
  .word UsageFault_Handler              /* Usage Fault Handler                 */
  .word 0                               /* Reserved                            */
  .word 0                               /* Reserved                            */
  .word 0                               /* Reserved                            */
  .word 0                               /* Reserved                            */
  .word SVC_Handler                     /* SVCall Handler                      */
  .word DebugMon_Handler                /* Debug Monitor Handler               */
  .word 0                               /* Reserved                            */
  .word PendSV_Handler                  /* PendSV Handler                      */
  .word SysTick_Handler                 /* SysTick Handler                     */

  /* External interrupts — STM32G474xx (IRQn 0–101) */
  .word WWDG_IRQHandler                /* 0: Window Watchdog                   */
  .word PVD_PVM_IRQHandler             /* 1: PVD/PVM through EXTI             */
  .word RTC_TAMP_LSECSS_IRQHandler     /* 2: RTC Tamper, TimeStamp, LSE CSS   */
  .word RTC_WKUP_IRQHandler            /* 3: RTC Wakeup through EXTI          */
  .word FLASH_IRQHandler               /* 4: Flash global                     */
  .word RCC_IRQHandler                 /* 5: RCC global                       */
  .word EXTI0_IRQHandler               /* 6: EXTI Line 0                      */
  .word EXTI1_IRQHandler               /* 7: EXTI Line 1                      */
  .word EXTI2_IRQHandler               /* 8: EXTI Line 2                      */
  .word EXTI3_IRQHandler               /* 9: EXTI Line 3                      */
  .word EXTI4_IRQHandler               /* 10: EXTI Line 4                     */
  .word DMA1_Channel1_IRQHandler       /* 11: DMA1 Channel 1                  */
  .word DMA1_Channel2_IRQHandler       /* 12: DMA1 Channel 2                  */
  .word DMA1_Channel3_IRQHandler       /* 13: DMA1 Channel 3                  */
  .word DMA1_Channel4_IRQHandler       /* 14: DMA1 Channel 4                  */
  .word DMA1_Channel5_IRQHandler       /* 15: DMA1 Channel 5                  */
  .word DMA1_Channel6_IRQHandler       /* 16: DMA1 Channel 6                  */
  .word DMA1_Channel7_IRQHandler       /* 17: DMA1 Channel 7                  */
  .word ADC1_2_IRQHandler              /* 18: ADC1 and ADC2                   */
  .word USB_HP_IRQHandler              /* 19: USB High Priority               */
  .word USB_LP_IRQHandler              /* 20: USB Low Priority                */
  .word FDCAN1_IT0_IRQHandler          /* 21: FDCAN1 IT0                      */
  .word FDCAN1_IT1_IRQHandler          /* 22: FDCAN1 IT1                      */
  .word EXTI9_5_IRQHandler             /* 23: EXTI Lines [9:5]                */
  .word TIM1_BRK_TIM15_IRQHandler      /* 24: TIM1 Break and TIM15           */
  .word TIM1_UP_TIM16_IRQHandler       /* 25: TIM1 Update and TIM16          */
  .word TIM1_TRG_COM_TIM17_IRQHandler  /* 26: TIM1 Trigger/Commut and TIM17  */
  .word TIM1_CC_IRQHandler             /* 27: TIM1 Capture Compare            */
  .word TIM2_IRQHandler                /* 28: TIM2                            */
  .word TIM3_IRQHandler                /* 29: TIM3                            */
  .word TIM4_IRQHandler                /* 30: TIM4                            */
  .word I2C1_EV_IRQHandler             /* 31: I2C1 Event                      */
  .word I2C1_ER_IRQHandler             /* 32: I2C1 Error                      */
  .word I2C2_EV_IRQHandler             /* 33: I2C2 Event                      */
  .word I2C2_ER_IRQHandler             /* 34: I2C2 Error                      */
  .word SPI1_IRQHandler                /* 35: SPI1                            */
  .word SPI2_IRQHandler                /* 36: SPI2                            */
  .word USART1_IRQHandler              /* 37: USART1                          */
  .word USART2_IRQHandler              /* 38: USART2                          */
  .word USART3_IRQHandler              /* 39: USART3                          */
  .word EXTI15_10_IRQHandler           /* 40: EXTI Lines [15:10]              */
  .word RTC_Alarm_IRQHandler           /* 41: RTC Alarm (A and B) via EXTI    */
  .word USBWakeUp_IRQHandler           /* 42: USB Wakeup via EXTI             */
  .word TIM8_BRK_IRQHandler            /* 43: TIM8 Break                      */
  .word TIM8_UP_IRQHandler             /* 44: TIM8 Update                     */
  .word TIM8_TRG_COM_IRQHandler        /* 45: TIM8 Trigger/Commutation        */
  .word TIM8_CC_IRQHandler             /* 46: TIM8 Capture Compare            */
  .word ADC3_IRQHandler                /* 47: ADC3                            */
  .word FMC_IRQHandler                 /* 48: FMC                             */
  .word LPTIM1_IRQHandler              /* 49: LPTIM1                          */
  .word TIM5_IRQHandler                /* 50: TIM5                            */
  .word SPI3_IRQHandler                /* 51: SPI3                            */
  .word UART4_IRQHandler               /* 52: UART4                           */
  .word UART5_IRQHandler               /* 53: UART5                           */
  .word TIM6_DAC_IRQHandler            /* 54: TIM6 and DAC1&3 underrun        */
  .word TIM7_DAC_IRQHandler            /* 55: TIM7 and DAC2&4 underrun        */
  .word DMA2_Channel1_IRQHandler       /* 56: DMA2 Channel 1                  */
  .word DMA2_Channel2_IRQHandler       /* 57: DMA2 Channel 2                  */
  .word DMA2_Channel3_IRQHandler       /* 58: DMA2 Channel 3                  */
  .word DMA2_Channel4_IRQHandler       /* 59: DMA2 Channel 4                  */
  .word DMA2_Channel5_IRQHandler       /* 60: DMA2 Channel 5                  */
  .word ADC4_IRQHandler                /* 61: ADC4                            */
  .word ADC5_IRQHandler                /* 62: ADC5                            */
  .word UCPD1_IRQHandler               /* 63: UCPD1                           */
  .word COMP1_2_3_IRQHandler           /* 64: COMP1, COMP2, COMP3             */
  .word COMP4_5_6_IRQHandler           /* 65: COMP4, COMP5, COMP6             */
  .word COMP7_IRQHandler               /* 66: COMP7                           */
  .word HRTIM1_Master_IRQHandler       /* 67: HRTIM Master Timer              */
  .word HRTIM1_TIMA_IRQHandler         /* 68: HRTIM Timer A                   */
  .word HRTIM1_TIMB_IRQHandler         /* 69: HRTIM Timer B                   */
  .word HRTIM1_TIMC_IRQHandler         /* 70: HRTIM Timer C                   */
  .word HRTIM1_TIMD_IRQHandler         /* 71: HRTIM Timer D                   */
  .word HRTIM1_TIME_IRQHandler         /* 72: HRTIM Timer E                   */
  .word HRTIM1_FLT_IRQHandler          /* 73: HRTIM Fault                     */
  .word HRTIM1_TIMF_IRQHandler         /* 74: HRTIM Timer F                   */
  .word CRS_IRQHandler                 /* 75: CRS                             */
  .word SAI1_IRQHandler                /* 76: SAI1                            */
  .word TIM20_BRK_IRQHandler           /* 77: TIM20 Break                     */
  .word TIM20_UP_IRQHandler            /* 78: TIM20 Update                    */
  .word TIM20_TRG_COM_IRQHandler       /* 79: TIM20 Trigger/Commutation       */
  .word TIM20_CC_IRQHandler            /* 80: TIM20 Capture Compare           */
  .word FPU_IRQHandler                 /* 81: FPU                             */
  .word I2C4_EV_IRQHandler             /* 82: I2C4 Event                      */
  .word I2C4_ER_IRQHandler             /* 83: I2C4 Error                      */
  .word SPI4_IRQHandler                /* 84: SPI4                            */
  .word 0                              /* 85: Reserved                        */
  .word FDCAN2_IT0_IRQHandler          /* 86: FDCAN2 IT0                      */
  .word FDCAN2_IT1_IRQHandler          /* 87: FDCAN2 IT1                      */
  .word FDCAN3_IT0_IRQHandler          /* 88: FDCAN3 IT0                      */
  .word FDCAN3_IT1_IRQHandler          /* 89: FDCAN3 IT1                      */
  .word RNG_IRQHandler                 /* 90: RNG                             */
  .word LPUART1_IRQHandler             /* 91: LPUART1                         */
  .word I2C3_EV_IRQHandler             /* 92: I2C3 Event                      */
  .word I2C3_ER_IRQHandler             /* 93: I2C3 Error                      */
  .word DMAMUX_OVR_IRQHandler          /* 94: DMAMUX overrun                  */
  .word QUADSPI_IRQHandler             /* 95: QUADSPI                         */
  .word DMA1_Channel8_IRQHandler       /* 96: DMA1 Channel 8                  */
  .word DMA2_Channel6_IRQHandler       /* 97: DMA2 Channel 6                  */
  .word DMA2_Channel7_IRQHandler       /* 98: DMA2 Channel 7                  */
  .word DMA2_Channel8_IRQHandler       /* 99: DMA2 Channel 8                  */
  .word CORDIC_IRQHandler              /* 100: CORDIC                         */
  .word FMAC_IRQHandler                /* 101: FMAC                           */

  .size g_pfnVectors, .-g_pfnVectors

/*******************************************************************************
 * Weak aliases — all handlers default to infinite loop unless overridden
 *******************************************************************************/
  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler
  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler
  .weak MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler
  .weak BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler
  .weak UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler
  .weak SVC_Handler
  .thumb_set SVC_Handler,Default_Handler
  .weak DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler
  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler
  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler
  .weak WWDG_IRQHandler
  .thumb_set WWDG_IRQHandler,Default_Handler
  .weak PVD_PVM_IRQHandler
  .thumb_set PVD_PVM_IRQHandler,Default_Handler
  .weak RTC_TAMP_LSECSS_IRQHandler
  .thumb_set RTC_TAMP_LSECSS_IRQHandler,Default_Handler
  .weak RTC_WKUP_IRQHandler
  .thumb_set RTC_WKUP_IRQHandler,Default_Handler
  .weak FLASH_IRQHandler
  .thumb_set FLASH_IRQHandler,Default_Handler
  .weak RCC_IRQHandler
  .thumb_set RCC_IRQHandler,Default_Handler
  .weak EXTI0_IRQHandler
  .thumb_set EXTI0_IRQHandler,Default_Handler
  .weak EXTI1_IRQHandler
  .thumb_set EXTI1_IRQHandler,Default_Handler
  .weak EXTI2_IRQHandler
  .thumb_set EXTI2_IRQHandler,Default_Handler
  .weak EXTI3_IRQHandler
  .thumb_set EXTI3_IRQHandler,Default_Handler
  .weak EXTI4_IRQHandler
  .thumb_set EXTI4_IRQHandler,Default_Handler
  .weak DMA1_Channel1_IRQHandler
  .thumb_set DMA1_Channel1_IRQHandler,Default_Handler
  .weak DMA1_Channel2_IRQHandler
  .thumb_set DMA1_Channel2_IRQHandler,Default_Handler
  .weak DMA1_Channel3_IRQHandler
  .thumb_set DMA1_Channel3_IRQHandler,Default_Handler
  .weak DMA1_Channel4_IRQHandler
  .thumb_set DMA1_Channel4_IRQHandler,Default_Handler
  .weak DMA1_Channel5_IRQHandler
  .thumb_set DMA1_Channel5_IRQHandler,Default_Handler
  .weak DMA1_Channel6_IRQHandler
  .thumb_set DMA1_Channel6_IRQHandler,Default_Handler
  .weak DMA1_Channel7_IRQHandler
  .thumb_set DMA1_Channel7_IRQHandler,Default_Handler
  .weak ADC1_2_IRQHandler
  .thumb_set ADC1_2_IRQHandler,Default_Handler
  .weak USB_HP_IRQHandler
  .thumb_set USB_HP_IRQHandler,Default_Handler
  .weak USB_LP_IRQHandler
  .thumb_set USB_LP_IRQHandler,Default_Handler
  .weak FDCAN1_IT0_IRQHandler
  .thumb_set FDCAN1_IT0_IRQHandler,Default_Handler
  .weak FDCAN1_IT1_IRQHandler
  .thumb_set FDCAN1_IT1_IRQHandler,Default_Handler
  .weak EXTI9_5_IRQHandler
  .thumb_set EXTI9_5_IRQHandler,Default_Handler
  .weak TIM1_BRK_TIM15_IRQHandler
  .thumb_set TIM1_BRK_TIM15_IRQHandler,Default_Handler
  .weak TIM1_UP_TIM16_IRQHandler
  .thumb_set TIM1_UP_TIM16_IRQHandler,Default_Handler
  .weak TIM1_TRG_COM_TIM17_IRQHandler
  .thumb_set TIM1_TRG_COM_TIM17_IRQHandler,Default_Handler
  .weak TIM1_CC_IRQHandler
  .thumb_set TIM1_CC_IRQHandler,Default_Handler
  .weak TIM2_IRQHandler
  .thumb_set TIM2_IRQHandler,Default_Handler
  .weak TIM3_IRQHandler
  .thumb_set TIM3_IRQHandler,Default_Handler
  .weak TIM4_IRQHandler
  .thumb_set TIM4_IRQHandler,Default_Handler
  .weak I2C1_EV_IRQHandler
  .thumb_set I2C1_EV_IRQHandler,Default_Handler
  .weak I2C1_ER_IRQHandler
  .thumb_set I2C1_ER_IRQHandler,Default_Handler
  .weak I2C2_EV_IRQHandler
  .thumb_set I2C2_EV_IRQHandler,Default_Handler
  .weak I2C2_ER_IRQHandler
  .thumb_set I2C2_ER_IRQHandler,Default_Handler
  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler
  .weak SPI2_IRQHandler
  .thumb_set SPI2_IRQHandler,Default_Handler
  .weak USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_Handler
  .weak USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Default_Handler
  .weak USART3_IRQHandler
  .thumb_set USART3_IRQHandler,Default_Handler
  .weak EXTI15_10_IRQHandler
  .thumb_set EXTI15_10_IRQHandler,Default_Handler
  .weak RTC_Alarm_IRQHandler
  .thumb_set RTC_Alarm_IRQHandler,Default_Handler
  .weak USBWakeUp_IRQHandler
  .thumb_set USBWakeUp_IRQHandler,Default_Handler
  .weak TIM8_BRK_IRQHandler
  .thumb_set TIM8_BRK_IRQHandler,Default_Handler
  .weak TIM8_UP_IRQHandler
  .thumb_set TIM8_UP_IRQHandler,Default_Handler
  .weak TIM8_TRG_COM_IRQHandler
  .thumb_set TIM8_TRG_COM_IRQHandler,Default_Handler
  .weak TIM8_CC_IRQHandler
  .thumb_set TIM8_CC_IRQHandler,Default_Handler
  .weak ADC3_IRQHandler
  .thumb_set ADC3_IRQHandler,Default_Handler
  .weak FMC_IRQHandler
  .thumb_set FMC_IRQHandler,Default_Handler
  .weak LPTIM1_IRQHandler
  .thumb_set LPTIM1_IRQHandler,Default_Handler
  .weak TIM5_IRQHandler
  .thumb_set TIM5_IRQHandler,Default_Handler
  .weak SPI3_IRQHandler
  .thumb_set SPI3_IRQHandler,Default_Handler
  .weak UART4_IRQHandler
  .thumb_set UART4_IRQHandler,Default_Handler
  .weak UART5_IRQHandler
  .thumb_set UART5_IRQHandler,Default_Handler
  .weak TIM6_DAC_IRQHandler
  .thumb_set TIM6_DAC_IRQHandler,Default_Handler
  .weak TIM7_DAC_IRQHandler
  .thumb_set TIM7_DAC_IRQHandler,Default_Handler
  .weak DMA2_Channel1_IRQHandler
  .thumb_set DMA2_Channel1_IRQHandler,Default_Handler
  .weak DMA2_Channel2_IRQHandler
  .thumb_set DMA2_Channel2_IRQHandler,Default_Handler
  .weak DMA2_Channel3_IRQHandler
  .thumb_set DMA2_Channel3_IRQHandler,Default_Handler
  .weak DMA2_Channel4_IRQHandler
  .thumb_set DMA2_Channel4_IRQHandler,Default_Handler
  .weak DMA2_Channel5_IRQHandler
  .thumb_set DMA2_Channel5_IRQHandler,Default_Handler
  .weak ADC4_IRQHandler
  .thumb_set ADC4_IRQHandler,Default_Handler
  .weak ADC5_IRQHandler
  .thumb_set ADC5_IRQHandler,Default_Handler
  .weak UCPD1_IRQHandler
  .thumb_set UCPD1_IRQHandler,Default_Handler
  .weak COMP1_2_3_IRQHandler
  .thumb_set COMP1_2_3_IRQHandler,Default_Handler
  .weak COMP4_5_6_IRQHandler
  .thumb_set COMP4_5_6_IRQHandler,Default_Handler
  .weak COMP7_IRQHandler
  .thumb_set COMP7_IRQHandler,Default_Handler
  .weak HRTIM1_Master_IRQHandler
  .thumb_set HRTIM1_Master_IRQHandler,Default_Handler
  .weak HRTIM1_TIMA_IRQHandler
  .thumb_set HRTIM1_TIMA_IRQHandler,Default_Handler
  .weak HRTIM1_TIMB_IRQHandler
  .thumb_set HRTIM1_TIMB_IRQHandler,Default_Handler
  .weak HRTIM1_TIMC_IRQHandler
  .thumb_set HRTIM1_TIMC_IRQHandler,Default_Handler
  .weak HRTIM1_TIMD_IRQHandler
  .thumb_set HRTIM1_TIMD_IRQHandler,Default_Handler
  .weak HRTIM1_TIME_IRQHandler
  .thumb_set HRTIM1_TIME_IRQHandler,Default_Handler
  .weak HRTIM1_FLT_IRQHandler
  .thumb_set HRTIM1_FLT_IRQHandler,Default_Handler
  .weak HRTIM1_TIMF_IRQHandler
  .thumb_set HRTIM1_TIMF_IRQHandler,Default_Handler
  .weak CRS_IRQHandler
  .thumb_set CRS_IRQHandler,Default_Handler
  .weak SAI1_IRQHandler
  .thumb_set SAI1_IRQHandler,Default_Handler
  .weak TIM20_BRK_IRQHandler
  .thumb_set TIM20_BRK_IRQHandler,Default_Handler
  .weak TIM20_UP_IRQHandler
  .thumb_set TIM20_UP_IRQHandler,Default_Handler
  .weak TIM20_TRG_COM_IRQHandler
  .thumb_set TIM20_TRG_COM_IRQHandler,Default_Handler
  .weak TIM20_CC_IRQHandler
  .thumb_set TIM20_CC_IRQHandler,Default_Handler
  .weak FPU_IRQHandler
  .thumb_set FPU_IRQHandler,Default_Handler
  .weak I2C4_EV_IRQHandler
  .thumb_set I2C4_EV_IRQHandler,Default_Handler
  .weak I2C4_ER_IRQHandler
  .thumb_set I2C4_ER_IRQHandler,Default_Handler
  .weak SPI4_IRQHandler
  .thumb_set SPI4_IRQHandler,Default_Handler
  .weak FDCAN2_IT0_IRQHandler
  .thumb_set FDCAN2_IT0_IRQHandler,Default_Handler
  .weak FDCAN2_IT1_IRQHandler
  .thumb_set FDCAN2_IT1_IRQHandler,Default_Handler
  .weak FDCAN3_IT0_IRQHandler
  .thumb_set FDCAN3_IT0_IRQHandler,Default_Handler
  .weak FDCAN3_IT1_IRQHandler
  .thumb_set FDCAN3_IT1_IRQHandler,Default_Handler
  .weak RNG_IRQHandler
  .thumb_set RNG_IRQHandler,Default_Handler
  .weak LPUART1_IRQHandler
  .thumb_set LPUART1_IRQHandler,Default_Handler
  .weak I2C3_EV_IRQHandler
  .thumb_set I2C3_EV_IRQHandler,Default_Handler
  .weak I2C3_ER_IRQHandler
  .thumb_set I2C3_ER_IRQHandler,Default_Handler
  .weak DMAMUX_OVR_IRQHandler
  .thumb_set DMAMUX_OVR_IRQHandler,Default_Handler
  .weak QUADSPI_IRQHandler
  .thumb_set QUADSPI_IRQHandler,Default_Handler
  .weak DMA1_Channel8_IRQHandler
  .thumb_set DMA1_Channel8_IRQHandler,Default_Handler
  .weak DMA2_Channel6_IRQHandler
  .thumb_set DMA2_Channel6_IRQHandler,Default_Handler
  .weak DMA2_Channel7_IRQHandler
  .thumb_set DMA2_Channel7_IRQHandler,Default_Handler
  .weak DMA2_Channel8_IRQHandler
  .thumb_set DMA2_Channel8_IRQHandler,Default_Handler
  .weak CORDIC_IRQHandler
  .thumb_set CORDIC_IRQHandler,Default_Handler
  .weak FMAC_IRQHandler
  .thumb_set FMAC_IRQHandler,Default_Handler
  .weak SystemInit
  .thumb_set SystemInit,Default_Handler
