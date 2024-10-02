// Copyright 2024 Accenture.

#include <platform/estdint.h>

#include <stdio.h>
#include <termios.h>

namespace
{
static char input = -1;

int internal_kbhit(void)
{
    struct termios org;
    int const fd = fileno(stdin);
    (void)tcgetattr(fd, &org);
    struct termios tmp = org;

    tmp.c_lflag &= ~(ICANON | ECHO);

    (void)tcsetattr(fd, TCSANOW, &tmp);
    int const chr = getchar();
    (void)tcsetattr(fd, TCSANOW, &org);

    input = chr;
    return chr != -1;
}

inline int internal_getch(void) { return input; }

} // namespace

extern "C" void putByteToStdout(uint8_t byte) { (void)printf("%c", byte); }

extern "C" int32_t getByteFromStdin()
{
    if (!internal_kbhit())
    {
        return -1;
    }

    int const byte = internal_getch();
    return byte ? byte : -1;
}
