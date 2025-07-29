#pragma once

#include <bsp/uart/UartConfig.h>

namespace bsp
{
struct Uart;

/**
 * This class implements the UART communication for S32K1xx platforms.
 * It is used as a base class for the Uart class.
 * The Uart class implements the methods for writing and reading data
 * over the UART interface.
 * The UartImpl provides methods specifics to the S32K1xx platform.
 */
class UartImpl : public bsp::UartConfig
{
public:
    /**
     * configures and starts the UART communication
     * this method must be called before using the read/write methods
     */
    void init();

    /**
     * it checks if there is active reading communication
     * @returns - true - there is no active reading data transfer
     */
    bool isRxReady() const;

protected:
    UartImpl(Id id);

    /**
     * it verifies if the transmission is completed and the hardware is ready for the next transfer
     * @returns - true - if tx transfer is active
     */
    bool isTxActive() const;

    /**
     * writes one byte of data to the UART interface.
     * @param data - the byte to be written
     * @return true if the byte was written successfully, false otherwise
     */
    bool writeByte(uint8_t data);

protected:
    struct UartDevice;
    UartDevice const& _uartDevice;
    static UartImpl::UartDevice const config_uart[NUMBER_OF_UARTS];
};

} // namespace bsp
