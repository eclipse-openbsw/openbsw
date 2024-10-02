// Copyright 2024 Accenture.

#ifndef GUARD_A91C2333_0583_4579_B986_AB77A649E20C
#define GUARD_A91C2333_0583_4579_B986_AB77A649E20C

#include "platform/estdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initializes the SCI device.
 */
void sciInit(void);
/**
 * Read actual init state
 */
uint8_t sciGetInitState(void);
/**
 * Write actual read process status
 * \return 1 -> ready, 0-> not ready
 */
uint8_t sciGetRxReady(void);
/**
 * Read one character from device
 * \return symbol
 */
uint8_t sciGeth(void);
/**
 * Read actual transfer status
 * \return 1 -> not ready, 0 -> ready
 */
uint8_t sciGetTxNotReady(void);
/**
 * Write one character to device
 */
void sciPuth(int c);

#ifdef __cplusplus
}
#endif

#endif /* GUARD_A91C2333_0583_4579_B986_AB77A649E20C */
