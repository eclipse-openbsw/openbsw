#pragma once

/**
 * Buffer size in bytes for asynchronous logger output
 */
#define CHARIOSERIAL_BUFFERSIZE 400

/**
 * Timeout for waiting serial console to get ready for sending,
 * in number of while-loop iterations
 */
#define SCI_LOGGERTIMEOUT 10000
