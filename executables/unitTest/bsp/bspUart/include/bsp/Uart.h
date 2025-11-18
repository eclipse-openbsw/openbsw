#pragma once

#include <bsp/uart/UartConcept.h>

namespace bsp
{
/**
 * This class implements the UART communication for the unit tests.
 */
class Uart
{
public:
    enum class Id : size_t;
    /**
     * Sends out an array of bytes.
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads an array of bytes.
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures and starts the UART communication.
     */
    void init();

    /**
     * Returns if this Uart instance is initialized or not.
     */
    bool isInitialized() const;

    /**
     * Waits until the UART is ready to transmit data.
     * @return true if ready within timeout, false otherwise
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
     * @param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id)
    {
        (void)id; // suppress unused parameter warning
        static Uart instance;
        return instance;
    }
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp
