// Copyright 2024 Accenture.

#pragma once

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
