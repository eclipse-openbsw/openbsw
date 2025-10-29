#pragma once

#include <bsp/UartId.h>
#include <bsp/uart/IUartApi.h>
#include <bsp/uart/UartConcept.h>

#include <termios.h>

namespace bsp
{
/**
 * This class implements the UART communication for the Posix platform.
 * It inherits from the IUartApi and UartId classes to provide the necessary
 * functionality and configuration for UART communication.
 * It uses the terminal stdout and stdin interfaces for reading and writing data.
 * The class provides methods to write and read data over the UART interface,
 * initialize the stdout/stdin communication, and retrieve a singleton instance of the Uart class.
 */
class Uart
: public bsp::IUartApi
, public bsp::UartId
{
public:
    /**
     * sends out a number of bytes
     * \param data - span of data to be sent
     * \return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * reads a number of bytes
     * \param data - span of data to be read
     * \return the number of bytes read from the terminal
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * configures and starts the terminal stdin/stdout communication
     * this method must be called before using the read/write methods
     */
    void init();

    /**
     * deinitializes the terminal stdin/stdout communication
     * this method should be called at the end of the application to clean up resources
     */
    void deinit();

    /**
     * checks if the stdin/stdout is initialized
     * \return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * waits until the stdin/stdout is ready to transmit data
     * \return true if ready within timeout, false otherwise
     */
    bool waitForTxReady();

    /**
     * factory method which instantiates and configures an UART object.
     * If the object exists it will returns only a reference to it.
     * \param id: TERMINAL, ...
     */
    static Uart& getInstance(Id id);

    Uart(Id id);

private:
    bool _initialized = false;
    int _std_out_fd{-1}; // File descriptor for stdout
    int _std_in_fd{-1};  // File descriptor for stdin

    struct termios _terminal_attr
    {}; // Terminal attributes for stdout
};

} // namespace bsp
