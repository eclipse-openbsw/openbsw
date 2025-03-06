
/*****************************************************************************/
/* Version: GCC for ARM Embedded Processors                                  */
/*****************************************************************************/
    .syntax unified
    .arch armv7-m

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                                      /* Top of Stack */
    .long   Reset_Handler                                   /* Reset Handler */
    .long   DefaultISR                                      /* NMI_Handler                                     /* NMI Handler*/
    .long   HardFault_Handler                               /* Hard Fault Handler*/
    .long   DefaultISR                                      /* MemManage_Handler                               /* MPU Fault Handler*/
    .long   BusFault_Handler                                /* Bus Fault Handler*/
    .long   UsageFault_Handler                              /* Usage Fault Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   SVC_Handler                                     /* SVCall Handler*/
    .long   DefaultISR                                      /* DebugMon_Handler                                /* Debug Monitor Handler*/
    .long   0                                               /* Reserved*/
    .long   PendSV_Handler                                  /* PendSV Handler*/
    .long   SysTick_Handler                                 /* SysTick Handler*/

                                                            /* External Interrupts*/
    .long   DefaultISR                                      /* DMA0_IRQHandler                                 /* DMA channel 0 transfer complete*/
    .long   DefaultISR                                      /* DMA1_IRQHandler                                 /* DMA channel 1 transfer complete*/
    .long   DefaultISR                                      /* DMA2_IRQHandler                                 /* DMA channel 2 transfer complete*/
    .long   DefaultISR                                      /* DMA3_IRQHandler                                 /* DMA channel 3 transfer complete*/
    .long   DefaultISR                                      /* DMA4_IRQHandler                                 /* DMA channel 4 transfer complete*/
    .long   DefaultISR                                      /* DMA5_IRQHandler                                 /* DMA channel 5 transfer complete*/
    .long   DefaultISR                                      /* DMA6_IRQHandler                                 /* DMA channel 6 transfer complete*/
    .long   DefaultISR                                      /* DMA7_IRQHandler                                 /* DMA channel 7 transfer complete*/
    .long   DefaultISR                                      /* DMA8_IRQHandler                                 /* DMA channel 8 transfer complete*/
    .long   DefaultISR                                      /* DMA9_IRQHandler                                 /* DMA channel 9 transfer complete*/
    .long   DefaultISR                                      /* DMA10_IRQHandler                                /* DMA channel 10 transfer complete*/
    .long   DefaultISR                                      /* DMA11_IRQHandler                                /* DMA channel 11 transfer complete*/
    .long   DefaultISR                                      /* DMA12_IRQHandler                                /* DMA channel 12 transfer complete*/
    .long   DefaultISR                                      /* DMA13_IRQHandler                                /* DMA channel 13 transfer complete*/
    .long   DefaultISR                                      /* DMA14_IRQHandler                                /* DMA channel 14 transfer complete*/
    .long   DefaultISR                                      /* DMA15_IRQHandler                                /* DMA channel 15 transfer complete*/
    .long   DefaultISR                                      /* DMA_Error_IRQHandler                            /* DMA error interrupt channels 0-15*/
    .long   DefaultISR                                      /* MCM_IRQHandler                                  /* FPU sources*/
    .long   DefaultISR                                      /* FTFC_IRQHandler                                 /* FTFC Command complete*/
    .long   DefaultISR                                      /* Read_Collision_IRQHandler                       /* FTFC Read collision*/
    .long   DefaultISR                                      /* LVD_LVW_IRQHandler                              /* PMC Low voltage detect interrupt*/
    .long   FTFC_Fault_IRQHandler                           /* FTFC Double bit fault detect*/
    .long   WDOG_EWM_IRQHandler                             /* WDOG_EWM_IRQHandler                             /* Single interrupt vector for WDOG and EWM*/
    .long   DefaultISR                                      /* RCM_IRQHandler                                  /* RCM Asynchronous Interrupt*/
    .long   DefaultISR                                      /* LPI2C0_Master_IRQHandler                        /* LPI2C0 Master Interrupt*/
    .long   DefaultISR                                      /* LPI2C0_Slave_IRQHandler                         /* LPI2C0 Slave Interrupt*/
    .long   DefaultISR                                      /* LPSPI0_IRQHandler                               /* LPSPI0 Interrupt*/
    .long   DefaultISR                                      /* LPSPI1_IRQHandler                               /* LPSPI1 Interrupt*/
    .long   DefaultISR                                      /* LPSPI2_IRQHandler                               /* LPSPI2 Interrupt*/
    .long   DefaultISR                                      /* Reserved45_IRQHandler                           /* Reserved Interrupt 45*/
    .long   DefaultISR                                      /* Reserved46_IRQHandler                           /* Reserved Interrupt 46*/
    .long   DefaultISR                                      /* UNUSED LPUART0 Transmit / Receive Interrupt*/
    .long   DefaultISR                                      /* Reserved48_IRQHandler                           /* Reserved Interrupt 48*/
    .long   DefaultISR                                      /* UNUSED LPUART1 Transmit / Receive Interrupt*/
    .long   DefaultISR                                      /* Reserved50_IRQHandler                           /* Reserved Interrupt 50*/
    .long   DefaultISR                                      /* UNUSED LPUART2 Transmit / Receive Interrupt*/
    .long   DefaultISR                                      /* Reserved52_IRQHandler                           /* Reserved Interrupt 52*/
    .long   DefaultISR                                      /* Reserved53_IRQHandler                           /* Reserved Interrupt 53*/
    .long   DefaultISR                                      /* Reserved54_IRQHandler                           /* Reserved Interrupt 54*/
    .long   DefaultISR                                      /* ADC0_IRQHandler                                 /* ADC0 interrupt request.*/
    .long   DefaultISR                                      /* ADC1_IRQHandler                                 /* ADC1 interrupt request.*/
    .long   DefaultISR                                      /* CMP0_IRQHandler                                 /* CMP0 interrupt request*/
    .long   DefaultISR                                      /* Reserved58_IRQHandler                           /* Reserved Interrupt 58*/
    .long   DefaultISR                                      /* Reserved59_IRQHandler                           /* Reserved Interrupt 59*/
    .long   DefaultISR                                      /* ERM_single_fault_IRQHandler                     /* ERM single bit error correction*/
    .long   ERM_double_fault_IRQHandler                     /* ERM double bit error non-correctable*/
    .long   DefaultISR                                      /* RTC_IRQHandler                                  /* RTC alarm interrupt*/
    .long   DefaultISR                                      /* RTC_Seconds_IRQHandler                          /* RTC seconds interrupt*/
    .long   DefaultISR                                      /* LPIT0_Ch0_IRQHandler                            /* LPIT0 channel 0 overflow interrupt*/
    .long   DefaultISR                                      /* LPIT0_Ch1_IRQHandler                            /* LPIT0 channel 1 overflow interrupt*/
    .long   DefaultISR                                      /* LPIT0_Ch2_IRQHandler                            /* LPIT0 channel 2 overflow interrupt*/
    .long   DefaultISR                                      /* LPIT0_Ch3_IRQHandler                            /* LPIT0 channel 3 overflow interrupt*/
    .long   DefaultISR                                      /* PDB0_IRQHandler                                 /* PDB0 interrupt*/
    .long   DefaultISR                                      /* Reserved69_IRQHandler                           /* Reserved Interrupt 69*/
    .long   DefaultISR                                      /* Reserved70_IRQHandler                           /* Reserved Interrupt 70*/
    .long   DefaultISR                                      /* Reserved71_IRQHandler                           /* Reserved Interrupt 71*/
    .long   DefaultISR                                      /* Reserved72_IRQHandler                           /* Reserved Interrupt 72*/
    .long   DefaultISR                                      /* SCG_IRQHandler                                  /* SCG bus interrupt request*/
    .long   DefaultISR                                      /* LPTMR0_IRQHandler                               /* LPTIMER interrupt request*/
    .long   DefaultISR                                      /* PORTA_IRQHandler                                /* Port A pin detect interrupt*/
    .long   DefaultISR                                      /* PORTB_IRQHandler                                /* Port B pin detect interrupt*/
    .long   DefaultISR                                      /* PORTC_IRQHandler                                /* Port C pin detect interrupt*/
    .long   DefaultISR                                      /* PORTD_IRQHandler                                /* Port D pin detect interrupt*/
    .long   DefaultISR                                      /* PORTE_IRQHandler                                /* Port E pin detect interrupt*/
    .long   DefaultISR                                      /* SWI_IRQHandler                                  /* Software interrupt*/
    .long   DefaultISR                                      /* Reserved81_IRQHandler                           /* Reserved Interrupt 81*/
    .long   DefaultISR                                      /* Reserved82_IRQHandler                           /* Reserved Interrupt 82*/
    .long   DefaultISR                                      /* Reserved83_IRQHandler                           /* Reserved Interrupt 83*/
    .long   DefaultISR                                      /* PDB1_IRQHandler                                 /* PDB1 interrupt*/
    .long   DefaultISR                                      /* FLEXIO_IRQHandler                               /* FlexIO Interrupt*/
    .long   DefaultISR                                      /* Reserved86_IRQHandler                           /* Reserved Interrupt 86*/
    .long   DefaultISR                                      /* Reserved87_IRQHandler                           /* Reserved Interrupt 87*/
    .long   DefaultISR                                      /* Reserved88_IRQHandler                           /* Reserved Interrupt 88*/
    .long   DefaultISR                                      /* Reserved89_IRQHandler                           /* Reserved Interrupt 89*/
    .long   DefaultISR                                      /* Reserved90_IRQHandler                           /* Reserved Interrupt 90*/
    .long   DefaultISR                                      /* Reserved91_IRQHandler                           /* Reserved Interrupt 91*/
    .long   DefaultISR                                      /* Reserved92_IRQHandler                           /* Reserved Interrupt 92*/
    .long   DefaultISR                                      /* Reserved93_IRQHandler                           /* Reserved Interrupt 93*/
    .long   DefaultISR                                      /* CAN0_ORed_IRQHandler                            /* CAN0 OR'ed [Bus Off OR Transmit Warning OR Receive Warning]*/
    .long   DefaultISR                                      /* CAN0_Error_IRQHandler                           /* CAN0 Interrupt indicating that errors were detected on the CAN bus*/
    .long   DefaultISR                                      /* CAN0_Wake_Up_IRQHandler                         /* CAN0 Interrupt asserted when Pretended Networking operation is enabled, and a valid message matches the selected filter criteria during Low Power mode*/
    .long   CAN0_ORed_0_15_MB_IRQHandler                    /* CAN0 OR'ed Message buffer (0-15)*/
    .long   CAN0_ORed_16_31_MB_IRQHandler                   /* CAN0 OR'ed Message buffer (16-31)*/
    .long   DefaultISR                                      /* Reserved99_IRQHandler                           /* Reserved Interrupt 99*/
    .long   DefaultISR                                      /* Reserved100_IRQHandler                          /* Reserved Interrupt 100*/
    .long   DefaultISR                                      /* CAN1_ORed_IRQHandler                            /* CAN1 OR'ed [Bus Off OR Transmit Warning OR Receive Warning]*/
    .long   DefaultISR                                      /* CAN1_Error_IRQHandler                           /* CAN1 Interrupt indicating that errors were detected on the CAN bus*/
    .long   DefaultISR                                      /* Reserved103_IRQHandler                          /* Reserved Interrupt 103*/
    .long   DefaultISR                                      /* CAN1_ORed_0_15_MB_IRQHandler                    /* CAN1 OR'ed Interrupt for Message buffer (0-15)*/
    .long   DefaultISR                                      /* CAN1_ORed_16_31_MB_IRQHandler                   /* CAN1 OR'ed Interrupt for Message buffer (16-31)*/
    .long   DefaultISR                                      /* Reserved106_IRQHandler                          /* Reserved Interrupt 106*/
    .long   DefaultISR                                      /* Reserved107_IRQHandler                          /* Reserved Interrupt 107*/
    .long   DefaultISR                                      /* CAN2_ORed_IRQHandler                            /* CAN2 OR'ed [Bus Off OR Transmit Warning OR Receive Warning]*/
    .long   DefaultISR                                      /* CAN2_Error_IRQHandler                           /* CAN2 Interrupt indicating that errors were detected on the CAN bus*/
    .long   DefaultISR                                      /* Reserved110_IRQHandler                          /* Reserved Interrupt 110*/
    .long   DefaultISR                                      /* CAN2_ORed_0_15_MB_IRQHandler                    /* CAN2 OR'ed Message buffer (0-15)*/
    .long   DefaultISR                                      /* Reserved112_IRQHandler                          /* Reserved Interrupt 112*/
    .long   DefaultISR                                      /* Reserved113_IRQHandler                          /* Reserved Interrupt 113*/
    .long   DefaultISR                                      /* Reserved114_IRQHandler                          /* Reserved Interrupt 114*/
    .long   DefaultISR                                      /* FTM0 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM0 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM0 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM0_Ch6_Ch7_IRQHandler                         /* FTM0 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM0_Fault_IRQHandler                           /* FTM0 Fault interrupt*/
    .long   DefaultISR                                      /* FTM0 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* FTM1_Ch0_Ch1_IRQHandler                         /* FTM1 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM1_Ch2_Ch3_IRQHandler                         /* FTM1 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM1_Ch4_Ch5_IRQHandler                         /* FTM1 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM1_Ch6_Ch7_IRQHandler                         /* FTM1 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM1_Fault_IRQHandler                           /* FTM1 Fault interrupt*/
    .long   DefaultISR                                      /* FTM1_Ovf_Reload_IRQHandler                      /* FTM1 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* FTM2_Ch0_Ch1_IRQHandler                         /* FTM2 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM2_Ch2_Ch3_IRQHandler                         /* FTM2 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM2_Ch4_Ch5_IRQHandler                         /* FTM2 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM2_Ch6_Ch7_IRQHandler                         /* FTM2 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM2_Fault_IRQHandler                           /* FTM2 Fault interrupt*/
    .long   DefaultISR                                      /* FTM2_Ovf_Reload_IRQHandler                      /* FTM2 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* FTM3_Ch0_Ch1_IRQHandler                         /* FTM3 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM3_Ch2_Ch3_IRQHandler                         /* FTM3 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM3_Ch4_Ch5_IRQHandler                         /* FTM3 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM3_Ch6_Ch7_IRQHandler                         /* FTM3 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM3_Fault_IRQHandler                           /* FTM3 Fault interrupt*/
    .long   DefaultISR                                      /* FTM3_Ovf_Reload_IRQHandler                      /* FTM3 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* FTM4_Ch0_Ch1_IRQHandler                         /* FTM4 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM4_Ch2_Ch3_IRQHandler                         /* FTM4 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM4_Ch4_Ch5_IRQHandler                         /* FTM4 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM4_Ch6_Ch7_IRQHandler                         /* FTM4 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM4_Fault_IRQHandler                           /* FTM4 Fault interrupt*/
    .long   DefaultISR                                      /* FTM4_Ovf_Reload_IRQHandler                      /* FTM4 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* FTM5_Ch0_Ch1_IRQHandler                         /* FTM5 Channel 0 and 1 interrupt*/
    .long   DefaultISR                                      /* FTM5_Ch2_Ch3_IRQHandler                         /* FTM5 Channel 2 and 3 interrupt*/
    .long   DefaultISR                                      /* FTM5_Ch4_Ch5_IRQHandler                         /* FTM5 Channel 4 and 5 interrupt*/
    .long   DefaultISR                                      /* FTM5_Ch6_Ch7_IRQHandler                         /* FTM5 Channel 6 and 7 interrupt*/
    .long   DefaultISR                                      /* FTM5_Fault_IRQHandler                           /* FTM5 Fault interrupt*/
    .long   DefaultISR                                      /* FTM5_Ovf_Reload_IRQHandler                      /* FTM5 Counter overflow and Reload interrupt*/
    .long   DefaultISR                                      /* 151*/
    .long   DefaultISR                                      /* 152*/
    .long   DefaultISR                                      /* 153*/
    .long   DefaultISR                                      /* 154*/
    .long   DefaultISR                                      /* 155*/
    .long   DefaultISR                                      /* 156*/
    .long   DefaultISR                                      /* 157*/
    .long   DefaultISR                                      /* 158*/
    .long   DefaultISR                                      /* 159*/
    .long   DefaultISR                                      /* 160*/
    .long   DefaultISR                                      /* 161*/
    .long   DefaultISR                                      /* 162*/
    .long   DefaultISR                                      /* 163*/
    .long   DefaultISR                                      /* 164*/
    .long   DefaultISR                                      /* 165*/
    .long   DefaultISR                                      /* 166*/
    .long   DefaultISR                                      /* 167*/
    .long   DefaultISR                                      /* 168*/
    .long   DefaultISR                                      /* 169*/
    .long   DefaultISR                                      /* 170*/
    .long   DefaultISR                                      /* 171*/
    .long   DefaultISR                                      /* 172*/
    .long   DefaultISR                                      /* 173*/
    .long   DefaultISR                                      /* 174*/
    .long   DefaultISR                                      /* 175*/
    .long   DefaultISR                                      /* 176*/
    .long   DefaultISR                                      /* 177*/
    .long   DefaultISR                                      /* 178*/
    .long   DefaultISR                                      /* 179*/
    .long   DefaultISR                                      /* 180*/
    .long   DefaultISR                                      /* 181*/
    .long   DefaultISR                                      /* 182*/
    .long   DefaultISR                                      /* 183*/
    .long   DefaultISR                                      /* 184*/
    .long   DefaultISR                                      /* 185*/
    .long   DefaultISR                                      /* 186*/
    .long   DefaultISR                                      /* 187*/
    .long   DefaultISR                                      /* 188*/
    .long   DefaultISR                                      /* 189*/
    .long   DefaultISR                                      /* 190*/
    .long   DefaultISR                                      /* 191*/
    .long   DefaultISR                                      /* 192*/
    .long   DefaultISR                                      /* 193*/
    .long   DefaultISR                                      /* 194*/
    .long   DefaultISR                                      /* 195*/
    .long   DefaultISR                                      /* 196*/
    .long   DefaultISR                                      /* 197*/
    .long   DefaultISR                                      /* 198*/
    .long   DefaultISR                                      /* 199*/
    .long   DefaultISR                                      /* 200*/
    .long   DefaultISR                                      /* 201*/
    .long   DefaultISR                                      /* 202*/
    .long   DefaultISR                                      /* 203*/
    .long   DefaultISR                                      /* 204*/
    .long   DefaultISR                                      /* 205*/
    .long   DefaultISR                                      /* 206*/
    .long   DefaultISR                                      /* 207*/
    .long   DefaultISR                                      /* 208*/
    .long   DefaultISR                                      /* 209*/
    .long   DefaultISR                                      /* 210*/
    .long   DefaultISR                                      /* 211*/
    .long   DefaultISR                                      /* 212*/
    .long   DefaultISR                                      /* 213*/
    .long   DefaultISR                                      /* 214*/
    .long   DefaultISR                                      /* 215*/
    .long   DefaultISR                                      /* 216*/
    .long   DefaultISR                                      /* 217*/
    .long   DefaultISR                                      /* 218*/
    .long   DefaultISR                                      /* 219*/
    .long   DefaultISR                                      /* 220*/
    .long   DefaultISR                                      /* 221*/
    .long   DefaultISR                                      /* 222*/
    .long   DefaultISR                                      /* 223*/
    .long   DefaultISR                                      /* 224*/
    .long   DefaultISR                                      /* 225*/
    .long   DefaultISR                                      /* 226*/
    .long   DefaultISR                                      /* 227*/
    .long   DefaultISR                                      /* 228*/
    .long   DefaultISR                                      /* 229*/
    .long   DefaultISR                                      /* 230*/
    .long   DefaultISR                                      /* 231*/
    .long   DefaultISR                                      /* 232*/
    .long   DefaultISR                                      /* 233*/
    .long   DefaultISR                                      /* 234*/
    .long   DefaultISR                                      /* 235*/
    .long   DefaultISR                                      /* 236*/
    .long   DefaultISR                                      /* 237*/
    .long   DefaultISR                                      /* 238*/
    .long   DefaultISR                                      /* 239*/
    .long   DefaultISR                                      /* 240*/
    .long   DefaultISR                                      /* 241*/
    .long   DefaultISR                                      /* 242*/
    .long   DefaultISR                                      /* 243*/
    .long   DefaultISR                                      /* 244*/
    .long   DefaultISR                                      /* 245*/
    .long   DefaultISR                                      /* 246*/
    .long   DefaultISR                                      /* 247*/
    .long   DefaultISR                                      /* 248*/
    .long   DefaultISR                                      /* 249*/
    .long   DefaultISR                                      /* 250*/
    .long   DefaultISR                                      /* 251*/
    .long   DefaultISR                                      /* 252*/
    .long   DefaultISR                                      /* 253*/
    .long   DefaultISR                                      /* 254*/
    .long   0xFFFFFFFF                                      /*  Reserved for user TRIM value*/

    .size    __isr_vector, . - __isr_vector

    .section .text // , "a" // has no effect for gcc compiler
    .equ _NVIC_ICER,  0xE000E180
    .equ _NVIC_ICPR,  0xE000E280
    .equ _NVIC_VTOR,  0xE000ED08
    .equ _CPACR,      0xE000ED88
    .text
    .thumb

/* Reset Handler */

    .thumb_func
    .align 2
    .globl   Reset_Handler
    .type    Reset_Handler, %function

Reset_Handler:

    cpsid   i               /* Mask interrupts */
    ldr r0, =_NVIC_ICER    /* Disable interrupts and clear pending flags */
    ldr r1, =_NVIC_ICPR
    ldr r2, =0xFFFFFFFF
    str r2, [r0]            /* NVIC_ICER - clear enable IRQ register 0x00 */
    str r2, [r1]            /* NVIC_ICPR - clear pending IRQ register  0x00*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x04*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x04*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x08*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x08*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x0c*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x0c*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x10*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x10*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x14*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x14*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x18*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x18*/

    str r2, [r0,#4]!            /* NVIC_ICER - clear enable IRQ register 0x1c*/
    str r2, [r1,#4]!            /* NVIC_ICPR - clear pending IRQ register 0x1c*/
/* ISR reloction */
    ISB
    DSB
    DMB
    ldr r0, =_NVIC_VTOR
    ldr r1, =__isr_vector
    str r1, [r0]
    ldr r0, [r1]
    mov sp, r0
    ldr r0, [r1, #4]
    ISB
    DSB
    DMB

    /* enable fpu */
    ldr r0, = _CPACR
    ldr r1, [r0]
    orr r1, r1, #0x00F00000
    str r1, [r0]

    /* Init the rest of the registers */
    ldr     r1,=0
    ldr     r2,=0
    ldr     r3,=0
    ldr     r4,=0
    ldr     r5,=0
    ldr     r6,=0
    ldr     r7,=0
    mov     r8,r7
    mov     r9,r7
    mov     r10,r7
    mov     r11,r7
    mov     r12,r7
    /* Initialize the stack pointer */
    ldr     r0,=__StackTop
    mov     r13,r0
/* c code call stack init .*/
    subs r0, 32
    str r1, [r0,r3]    /*0*/
    movs    r3, #4
    str r1, [r0,r3]    /*4*/
    add r0, r0, r3
    str r1, [r0,r3]    /*8*/
    add r0, r0, r3
    str r1, [r0,r3]    /*12*/
    add r0, r0, r3
    str r1, [r0,r3]    /*16*/
    add r0, r0, r3
    str r1, [r0,r3]    /*20*/
    add r0, r0, r3
    str r1, [r0,r3]    /*24*/
    add r0, r0, r3
    str r1, [r0,r3]    /*28*/

    bl boardInit

    cpsie   i               /* Unmask interrupts */

    /* Init ECC RAM */

    ldr r1, =__DATA_RAM
    ldr r2, =__DATA_RAM_END

    subs    r2, r1
    ble .LC5

    movs    r0, 0
    movs    r3, #4
.LC4:
    str r0, [r1]
    add r1, r1, r3
    subs r2, 4
    bgt .LC4
.LC5:

/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs    r3, r2
    ble     .LC0

.LC1:
    subs    r3, 4
    ldr    r0, [r1,r3]
    str    r0, [r2,r3]
    bgt    .LC1
.LC0:

/*     This part of work usually is done in C library startup code. Otherwise,
 *     define this macro to enable it in this startup.
 *
 *     Loop to zero out BSS section, which uses following symbols
 *     in linker script:
 *      __bss_start__: start of BSS section. Must align to 4
 *      __bss_end__: end of BSS section. Must align to 4
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    subs    r2, r1
    ble .LC3

    movs    r0, 0
.LC2:
    str r0, [r1, r2]
    subs    r2, 4
    bge .LC2
.LC3:

/*
 * Zero out the MPU protected BSS section
 */
    ldr r1, =__MPU_BSS_START
    ldr r2, =__MPU_BSS_END

    subs    r2, r1
    ble .LC7

    movs    r0, 0
.LC6:
    str r0, [r1, r2]
    subs    r2, 4
    bge .LC6
.LC7:

#ifndef __START
#define __START _start
#endif
	bl    __libc_init_array

    bl    main /*__START*/
    .pool
    .size Reset_Handler, . - Reset_Handler


    .align  1
    .thumb_func
    .weak DefaultISR
    .type DefaultISR, %function
DefaultISR:
    b       DefaultISR
    .size DefaultISR, . - DefaultISR

Exception:
     cpsid   i               /* Mask interrupts */
     b    Exception

    .section .__startUpSection, "a"
    .thumb_func
    .align 2
    .globl   __start
__start:
    ldr r2, =Reset_Handler
    bx  r2

.end
