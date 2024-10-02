// Copyright 2024 Accenture.

#include "util/StdIoMock.h"

using namespace ::util::test;

extern "C"
{
int32_t getByteFromStdin()
{
    int32_t const ret = StdIoMock::instance().in.size() != 0 ? StdIoMock::instance().in[0] : -1;
    StdIoMock::instance().in.advance(1);
    return ret;
}

void putByteToStdout(uint8_t b) { StdIoMock::instance().out.push_back(b); }
} /* extern "C" */
