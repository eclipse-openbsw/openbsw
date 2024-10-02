#ifndef CHARIOSERIAL_H_
#define CHARIOSERIAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Make logging synchronous or asynchronous
 */
void SerialLogger_setAsynchron(void);
void SerialLogger_setSynchron(void);

void SerialLogger_init(void);

/**
 * Prints an ASCII character on the serial console
 * \param  c The character to be printed
 */
int SerialLogger_putc(int c);

/**
 * Get incoming characters from the serial console
 * \return
 * - 0 if no characters available
 * - the received character, if any
 */
int SerialLogger_getc(void);

/**
 * Idle task to output the printf buffer
 */
void SerialLogger_Idle(void);

/**
 * Standard library replacement for printing a character
 */
int SerialLogger__outchar(int c, int last);

/**
 * Standard library replacement for reading a raw character from port
 */
int SerialLogger__inchar(void);

#ifdef __cplusplus
}
#endif

#endif /* CHARIOSERIAL_H_ */
