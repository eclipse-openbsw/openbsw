// Copyright 2024 Accenture.

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSP_IO_PIN_CONFIGURATION) && (BSP_IO_PIN_CONFIGURATION == 1)

Io::PinConfiguration const Io::fPinConfiguration[Io::NUMBER_OF_IOS] = {

    /* 00  */ {_PORTA_, PA0, _IN, FILTER_ACTIVE | FILTER_TICK1, GPIO},
    /* 01  */ {_PORTA_, PA1, _OUT, FILTER_ACTIVE | FILTER_TICK1, GPIO | STRENGTH_ON},
    /* 02  */ {_PORTA_, PA2, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 03  */ {_PORTA_, PA3, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 04  */ {_PORTA_, PA4, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 05  */ {_PORTA_, PA5, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 06  */ {_PORTA_, PA6, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 07  */ {_PORTA_, PA7, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 08  */ {_PORTA_, PA8, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 09  */ {_PORTA_, PA9, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 10  */ {_PORTA_, PA10, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 11  */ {_PORTA_, PA11, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 12  */ {_PORTA_, PA12, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 13  */ {_PORTA_, PA13, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 14  */ {_PORTA_, PA14, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 15  */ {_PORTA_, PA15, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 16  */ {_PORTA_, PA16, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 17  */ {_PORTA_, PA17, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},

    /* 18  */ {_PORTB_, PB0, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 19  */ {_PORTB_, PB1, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 20  */ {_PORTB_, PB2, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 21  */ {_PORTB_, PB3, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 22  */ {_PORTB_, PB4, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 23  */ {_PORTB_, PB5, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 24  */ {_PORTB_, PB6, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 25  */ {_PORTB_, PB7, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 26  */ {_PORTB_, PB8, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 27  */ {_PORTB_, PB9, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 28  */ {_PORTB_, PB10, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 29  */ {_PORTB_, PB11, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 30  */ {_PORTB_, PB12, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 31  */ {_PORTB_, PB13, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 32  */ {_PORTB_, PB14, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 33  */ {_PORTB_, PB15, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 34  */ {_PORTB_, PB16, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 35  */ {_PORTB_, PB17, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},

    /* 36  */ {_PORTC_, PC0, _IN, FILTER_ACTIVE | FILTER_TICK1, ALT3},
    /* 37  */ {_PORTC_, PC1, _OUT, FILTER_ACTIVE | FILTER_TICK1, ALT3 | STRENGTH_ON},
    /* 38  */ {_PORTC_, PC2, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 39  */ {_PORTC_, PC3, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 40  */ {_PORTC_, PC4, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 41  */ {_PORTC_, PC5, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 42  */ {_PORTC_, PC6, _IN, 0, ALT2},
    /* 43  */ {_PORTC_, PC7, _OUT, 0, ALT2 | STRENGTH_ON},
    /* 44  */ {_PORTC_, PC8, _IN, 0, ALT2},
    /* 45  */ {_PORTC_, PC9, _OUT, 0, ALT2 | STRENGTH_ON},
    /* 46  */ {_PORTC_, PC10, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 47  */ {_PORTC_, PC11, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 48  */ {_PORTC_, PC12, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 49  */ {_PORTC_, PC13, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 50  */ {_PORTC_, PC14, _OUT, FILTER_ACTIVE | FILTER_TICK1, GPIO | STRENGTH_ON},
    /* 51  */ {_PORTC_, PC15, _OUT, FILTER_ACTIVE | FILTER_TICK1, ALT3 | STRENGTH_ON},
    /* 52  */ {_PORTC_, PC16, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 53  */ {_PORTC_, PC17, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 54  */ {_PORTC_, PC28, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},

    /* 55  */ {_PORTD_, PD0, _OUT, 0, ALT3 | STRENGTH_ON | PULLUP},
    /* 56  */ {_PORTD_, PD1, _IN, 0, ALT3 | PULLUP},
    /* 57  */ {_PORTD_, PD2, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 58  */ {_PORTD_, PD3, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 59  */ {_PORTD_, PD4, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 61  */ {_PORTD_, PD5, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 62  */ {_PORTD_, PD6, _IN, 0, ALT2},
    /* 63  */ {_PORTD_, PD7, _OUT, 0, ALT2 | STRENGTH_ON},
    /* 64  */ {_PORTD_, PD8, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 65  */ {_PORTD_, PD9, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 66  */ {_PORTD_, PD10, _OUT, FILTER_ACTIVE | FILTER_TICK1, GPIO | STRENGTH_ON | PULLUP},
    /* 67  */ {_PORTD_, PD11, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 68  */ {_PORTD_, PD12, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 69  */ {_PORTD_, PD13, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 70  */ {_PORTD_, PD14, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 71  */ {_PORTD_, PD15, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 72  */ {_PORTD_, PD16, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 73  */ {_PORTD_, PD17, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},

    /* 74  */ {_PORTE_, PE0, _OUT, 0, ALT5 | STRENGTH_ON},
    /* 75  */ {_PORTE_, PE1, _OUT, 0, GPIO | STRENGTH_ON | PULLUP},
    /* 76  */ {_PORTE_, PE2, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 77  */ {_PORTE_, PE3, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 78  */ {_PORTE_, PE4, _IN, 0, ALT5},
    /* 79  */ {_PORTE_, PE5, _OUT, 0, ALT5 | STRENGTH_ON},
    /* 80  */ {_PORTE_, PE6, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 81  */ {_PORTE_, PE7, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 82  */ {_PORTE_, PE8, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 83  */ {_PORTE_, PE9, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 84  */ {_PORTE_, PE10, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 85  */ {_PORTE_, PE11, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 86  */ {_PORTE_, PE12, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 87  */ {_PORTE_, PE13, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 88  */ {_PORTE_, PE14, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 89  */ {_PORTE_, PE15, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 90  */ {_PORTE_, PE16, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
    /* 91  */ {_PORTE_, PE21, _OUT, FILTER_ACTIVE | FILTER_TICK1, ALT2 | STRENGTH_ON},
    /* 92  */ {_PORTE_, PE22, _OUT, FILTER_ACTIVE | FILTER_TICK1, ALT2 | STRENGTH_ON},
    /* 93  */ {_PORTE_, PE23, _OUT, FILTER_ACTIVE | FILTER_TICK1, ALT2 | STRENGTH_ON},

    /* 94  */ {_PORTC_, PC29, _IN, FILTER_ACTIVE | FILTER_TICK1, PINDISABLE},
};

#else

enum PinId
{
    /* 00 */ EVAL_DI_1,
    /* 01 */ EVAL_DO_1,
    /* 02 */ EVAL_AI_1,
    /* 03 */ PIN_A_3,
    /* 04 */ PIN_A_4,
    /* 05 */ PIN_A_5,
    /* 06 */ PIN_A_6,
    /* 07 */ PIN_A_7,
    /* 08 */ PIN_A_8,
    /* 09 */ PIN_A_9,
    /* 10 */ PIN_A_10,
    /* 11 */ PIN_A_11,
    /* 12 */ PIN_A_12,
    /* 13 */ PIN_A_13,
    /* 14 */ PIN_A_14,
    /* 15 */ PIN_A_15,
    /* 16 */ PIN_A_16,
    /* 17 */ PIN_A_17,

    /* 18 */ PIN_B_0,
    /* 19 */ PIN_B_1,
    /* 20 */ PIN_B_2,
    /* 21 */ PIN_B_3,
    /* 22 */ PIN_B_4,
    /* 23 */ PIN_B_5,
    /* 24 */ PIN_B_6,
    /* 25 */ PIN_B_7,
    /* 26 */ PIN_B_8,
    /* 27 */ PIN_B_9,
    /* 28 */ PIN_B_10,
    /* 29 */ PIN_B_11,
    /* 30 */ PIN_B_12,
    /* 31 */ PIN_B_13,
    /* 32 */ PIN_B_14,
    /* 33 */ PIN_B_15,
    /* 34 */ PIN_B_16,
    /* 35 */ PIN_B_17,

    /* 36 */ SPI2_MISO,
    /* 37 */ SPI2_MOSI,
    /* 38 */ PIN_C_2,
    /* 39 */ PIN_C_3,
    /* 40 */ PIN_C_4,
    /* 41 */ PIN_C_5,
    /* 42 */ UART1_RX,
    /* 43 */ UART1_TX,
    /* 44 */ LIN1_Rx,
    /* 45 */ LIN1_Tx,
    /* 46 */ PIN_C_10,
    /* 47 */ PIN_C_11,
    /* 48 */ PIN_C_12,
    /* 49 */ PIN_C_13,
    /* 50 */ SPI2_CSN0,
    /* 51 */ SPI2_SCK,
    /* 52 */ PIN_C_16,
    /* 53 */ PIN_C_17,
    /* 54 */ EVAL_POTI_ADC,

    /* 55 */ SPI1_SCK,
    /* 56 */ SPI1_MISO,
    /* 57 */ PIN_D_2,
    /* 58 */ PIN_D_3,
    /* 59 */ PIN_D_4,
    /* 61 */ PIN_D_5,
    /* 62 */ UART_RX,
    /* 63 */ UART_TX,
    /* 64 */ PIN_D_8,
    /* 65 */ PIN_D_9,
    /* 66 */ SPI1_CSN1,
    /* 67 */ PIN_D_11,
    /* 68 */ PIN_D_12,
    /* 69 */ PIN_D_13,
    /* 70 */ PIN_D_14,
    /* 71 */ PIN_D_15,
    /* 72 */ PIN_D_16,
    /* 73 */ PIN_D_17,

    /* 74 */ SPI1_MOSI,
    /* 75 */ SPI1_CSN0,
    /* 76 */ PIN_E_2,
    /* 77 */ PIN_E_3,
    /* 78 */ canRx,
    /* 79 */ canTx,
    /* 80 */ PIN_E_6,
    /* 81 */ PIN_E_7,
    /* 82 */ PIN_E_8,
    /* 83 */ PIN_E_9,
    /* 84 */ PIN_E_10,
    /* 85 */ PIN_E_11,
    /* 86 */ PIN_E_12,
    /* 87 */ PIN_E_13,
    /* 88 */ PIN_E_14,
    /* 89 */ PIN_E_15,
    /* 90 */ PIN_E_16,
    /* 91 */ EVAL_LED_RED,
    /* 92 */ EVAL_LED_GREEN,
    /* 93 */ EVAL_LED_BLUE,
    /* 94 */ EVAL_ADC,

    /* xx */ NUMBER_OF_INPUTS_AND_OUTPUTS,
    /* xx */ PORT_UNAVAILABLE = NUMBER_OF_INPUTS_AND_OUTPUTS,
};

#endif /* BSP_IO_PIN_CONFIGURATION == 1 */
