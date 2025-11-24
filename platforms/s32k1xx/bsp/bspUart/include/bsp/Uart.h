#pragma once

#include <bsp/uart/UartConcept.h>

namespace bsp
{
/**
 * This class implements the UART communication for S32K1xx platforms.
 * It follows the method signatures defined in the UartConcept.h file.
 */
class Uart
{
public:
    /*
     * Enum for identifying different UART instances (e.g., TERMINAL).
     */
    enum class Id : size_t;
    /**
     * Sends out a number of bytes over the UART interface.
     * The method will block until the data is sent.
     * \param data - span of data to be sent
     * \return the number of bytes written to the uart interface
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads a number of bytes over the UART interface.
     * The method will block until the data is read.
     * \param data - the span where the data will be read
     * \return the number of bytes read from the uart interface
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures and starts the UART communication.
     * This method must be called before using the read/write methods.
     */
    void init();

    /**
     * Returns if this Uart instance is initialized or not.
     */
    bool isInitialized() const;

    /**
     * Waits until the UART is ready to transmit data.
     * \return true if the Uart is ready for transmission, false otherwise
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
     * \param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

    struct UartConfig;
    Uart(Uart::Id id);

private:
    /**
     * Writes one byte of data to the UART interface.
     * \param data - the byte to be written
     * \return true if the byte was written successfully, false otherwise
     */
    bool writeByte(uint8_t data);

private:
    UartConfig const& _uartConfig;
    static UartConfig const _uartConfigs[];
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp
