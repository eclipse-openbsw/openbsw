#pragma once

#include <bsp/UartId.h>
#include <bsp/uart/IUartApi.h>
#include <bsp/uart/UartConcept.h>

namespace bsp
{
/**
 * This class implements the UART communication for the unit tests.
 */
class Uart
: public bsp::IUartApi
, public bsp::UartId
{
public:
    /**
     * sends out an array of bytes .
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * reads an array of bytes.
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * configures and starts the UART communication
     */
    void init();

    /**
     * checks if the UART is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * waits until the UART is ready to transmit data
     * @return true if ready within timeout, false otherwise
     */
    bool waitForTxReady();

    /**
     * factory method which instantiates and configures an UART object.
     * If the object exists it will returns only a reference to it.
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
