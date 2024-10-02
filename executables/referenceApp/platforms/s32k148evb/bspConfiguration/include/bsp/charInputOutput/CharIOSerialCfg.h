// Copyright 2024 Accenture.

#ifndef GUARD_B8C42E07_E817_4620_B352_EA2FE788AFA0
#define GUARD_B8C42E07_E817_4620_B352_EA2FE788AFA0

/**
 * Asynchronous buffer size for logger output in bytes
 */
#ifndef __DEBUG_ANIMATION
#define CHARIOSERIAL_BUFFERSIZE 256
#else
#define CHARIOSERIAL_BUFFERSIZE 100
#endif

/**
 * Timeout for busy wait
 */
#define SCI_LOGGERTIMEOUT 10000

#endif /* GUARD_B8C42E07_E817_4620_B352_EA2FE788AFA0 */
