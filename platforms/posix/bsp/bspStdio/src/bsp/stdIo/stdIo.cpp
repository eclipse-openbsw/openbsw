// Copyright 2024 Accenture.

#include <platform/estdint.h>

#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static bool terminal_setup_done = false;
static int terminal_stdout_fd   = -1;
static struct termios terminal_original_attr;

void terminal_setup(void)
{
    if (!terminal_setup_done)
    {
        terminal_stdout_fd = fileno(stdout);
        if (terminal_stdout_fd == -1)
        {
            return;
        }
        (void)tcgetattr(terminal_stdout_fd, &terminal_original_attr);
        struct termios tmp = terminal_original_attr;
        tmp.c_lflag &= ~(ICANON | ECHO);
        (void)tcsetattr(terminal_stdout_fd, TCSANOW, &tmp);
        terminal_setup_done = true;
    }
    return;
}

void terminal_cleanup(void)
{
    if (terminal_setup_done)
    {
        (void)tcsetattr(terminal_stdout_fd, TCSANOW, &terminal_original_attr);
        terminal_setup_done = false;
    }
}

extern "C" void putByteToStdout(uint8_t byte)
{
    if (terminal_setup_done)
    {
        while (write(terminal_stdout_fd, &byte, 1) != 1)
        {
            // Only if write is interrupted by a signal before it writes any data
            // then try to write the byte again
            if (errno != EINTR)
            {
                break;
            }
        }
    }
}

extern "C" int32_t getByteFromStdin()
{
    if (terminal_setup_done)
    {
        return getchar();
    }
    return -1;
}
