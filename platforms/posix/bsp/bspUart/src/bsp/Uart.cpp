#include <bsp/Uart.h>
#include <bsp/uart/UartConfig.h>

#include <errno.h>
#include <fcntl.h> // For fcntl and O_NONBLOCK
#include <stdio.h>
#include <unistd.h>

using bsp::Uart;

Uart::Uart(Id id) { (void)id; }

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    size_t bytes_written = 0;
    if (_initialized)
    {
        ssize_t result = 0;
        do
        {
            result = ::write(_std_out_fd, data.data(), data.size());
            if (result > 0)
            {
                bytes_written = static_cast<size_t>(result);
            }
        } while (result == -1 && errno == EINTR);
    }
    return bytes_written;
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytes_read = 0;
    if (_initialized)
    {
        bytes_read = ::read(_std_in_fd, data.data(), data.size());
    }
    return bytes_read;
}

void Uart::init()
{
    if (!_initialized)
    {
        int oldflags;
        _std_out_fd = fileno(stdout);
        if (_std_out_fd == -1)
        {
            return;
        }
        _std_in_fd = fileno(stdin);
        if (_std_in_fd == -1)
        {
            return;
        }
        (void)tcgetattr(_std_out_fd, &_terminal_attr);
        struct termios tmp = _terminal_attr;
        tmp.c_lflag &= ~(ICANON | ECHO);
        (void)tcsetattr(_std_out_fd, TCSANOW, &tmp);

        // Set the file descriptor to non-blocking
        oldflags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldflags | O_NONBLOCK);
        _initialized = true;
    }
}

void Uart::deinit()
{
    if (_initialized)
    {
        (void)tcsetattr(_std_out_fd, TCSANOW, &_terminal_attr);
        _initialized = false;
    }
}

bsp::Uart& Uart::getInstance(Id id)
{
    static Uart instances[] = {
        Uart(Uart::Id::TERMINAL),
    };

    ETL_ASSERT(
        id < Id::INVALID, ETL_ERROR_GENERIC("UartId::INVALID is not a valid Uart identifier"));

    static_assert(NUMBER_OF_UARTS == etl::size(instances), "Not enough Uart instances defined");
    return instances[static_cast<size_t>(id)];
}

bool Uart::isInitialized() const { return _initialized; }

bool Uart::waitForTxReady() { return true; }
