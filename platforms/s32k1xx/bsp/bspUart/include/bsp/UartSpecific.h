#pragma once

#include <bsp/uart/UartConfig.h>
#include <etl/span.h>

namespace bsp
{
class Uart;

/**
 * This class implements the UART communication for S32K1xx platforms.
 * It is used as a base class for the Uart class.
 * The Uart class implements the methods for writing and reading data
 * over the UART interface.
 * The UartSpecific provides methods specifics to the S32K1xx platform.
 */
class UartSpecific : public bsp::UartConfig
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

    /**
     * factory method which instantiates and configures an UART object.
     * If the object exists it will returns only a reference to it.
     * @param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

protected:
    UartSpecific(Id id);

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
    static UartSpecific::UartDevice const config_uart[NUMBER_OF_UARTS];
};

} // namespace bsp
