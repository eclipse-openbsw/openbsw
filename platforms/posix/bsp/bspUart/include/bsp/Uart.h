#pragma once

#include <bsp/uart/UartConcept.h>

#include <termios.h>

namespace bsp
{
/**
 * This class implements the UART communication for the Posix platform.
 * It uses the terminal stdout and stdin interfaces for reading and writing data.
 * Initialize the stdout/stdin communication, and retrieve a singleton instance of the Uart class.
 */
class Uart
{
public:
    enum class Id : size_t;

    /**
     * Sends out a number of bytes.
     * \param data - span of data to be sent
     * \return the number of bytes written
     */
    size_t write(::etl::span<uint8_t const> const data);

    /**
     * Reads a number of bytes.
     * \param data - span of data to be read
     * \return the number of bytes read from the terminal
     */
    size_t read(::etl::span<uint8_t> data);

    /**
     * Configures and starts the terminal stdin/stdout communication.
     * This method must be called before using the read/write methods.
     */
    void init();

    /**
     * Deinitializes the terminal stdin/stdout communication.
     * This method should be called at the end of the application to clean up resources.
     */
    void deinit();

    /**
     * Returns if this Uart instance is initialized or not.
     */
    bool isInitialized() const;

    /**
     * Waits until the stdin/stdout is ready to transmit data.
     * \return true if ready within timeout, false otherwise
     */
    bool waitForTxReady();

    /**
     * Returns the singleton instance of the Uart object.
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
