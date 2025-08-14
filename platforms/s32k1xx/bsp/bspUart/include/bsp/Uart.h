#pragma once

#include <bsp/uart/UartApi.h>
#include <bsp/uart/UartConfig.h>

namespace bsp
{
/**
 * This class implements the UART communication for S32K1xx platforms.
 * It inherits from the base class UartApi and implements the methods for writing and reading data
 * over the UART interface. It also provides a method for initializing the UART communication.
 */
class Uart
: public bsp::UartApi
, public bsp::UartConfig
{
public:
    /**
     * sends out an array of bytes over the UART interface.
     * the method will block until the data is sent.
     * @param data - pointer to the data to be sent
     *        length - the number of bytes to be sent
     * @return the number of bytes written to the uart interface
     */
    size_t write(uint8_t const* data, size_t length);

    /**
     * reads an array of bytes over the UART interface.
     * the method will block until the data is read.
     * @param data - pointer to the data to be read
     *        length - the number of bytes to be read
     * @return the number of bytes read from the uart interface
     */
    size_t read(uint8_t* data, size_t length);

    /**
     * configures and starts the UART communication
     * this method must be called before using the read/write methods
     */
    void init();

    /**
     * factory method which instantiates and configures an UART object.
     * If the object exists it will returns only a reference to it.
     * @param id: TERMINAL, ...
     */
    static Uart& get(Id id);

    /**
     * it checks if there is active reading communication
     * @returns - true - there is no active reading data transfer
     */
    bool isRxReady() const;

private:
    struct UartDevice;

    Uart(UartDevice const& uartDevice) : _uartDevice(uartDevice) {}

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

private:
    UartDevice const& _uartDevice;
    static Uart::UartDevice const config_uart[NUMBER_OF_UARTS];
};

} // namespace bsp
