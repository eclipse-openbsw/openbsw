// Copyright 2024 Accenture.

#ifndef GUARD_93DA54A2_4708_4F9A_921C_E547CD648629
#define GUARD_93DA54A2_4708_4F9A_921C_E547CD648629

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void sysDelayUs(uint32_t delay);

/**
 * Returns the number of Ticks since startup of the CPU.
 * \return Systemtime in Ticks.
 */
uint64_t getSystemTicks(void);

/**
 * Returns the number of Ticks since startup of the CPU.
 * \return Systemtime in Ticks.
 * Fast call for difference calculations, overruns might occur
 */
uint32_t getSystemTicks32Bit(void);

/*
 * Returns the Time in Us with overrun
 * \return systemTime in mksec
 */
uint32_t getSystemTimeUs32Bit(void);

/*
 * Returns the Time in ms with overrun
 * \return systemTime in millisec
 */
uint32_t getSystemTimeMs32Bit(void);

/**
 * Returns the Time in Ns since startup of the CPU.
 * \return Systemtime in Ns.
 */
uint64_t getSystemTimeNs(void);

/**
 * Returns the Time in Us since startup of the CPU.
 * \return Systemtime in Us.
 */
uint64_t getSystemTimeUs(void);

/**
 * Returns the Time in Ms since startup of the CPU.
 * \return Systemtime in Ms.
 */
uint64_t getSystemTimeMs(void);

/**
 * Returns the converted value in Us from a given value in system ticks.
 */
uint64_t systemTicksToTimeUs(uint64_t ticks);

/**
 * Returns the converted value in Ns from a given value in system ticks.
 */
uint64_t systemTicksToTimeNs(uint64_t ticks);

void initSystemTimer();
/*
 * Returns project dependent TickTime
 */
uint32_t getSystemTime32BitUserTicks(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GUARD_93DA54A2_4708_4F9A_921C_E547CD648629 */
