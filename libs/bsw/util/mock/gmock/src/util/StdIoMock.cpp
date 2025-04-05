// Copyright 2024 Accenture.

#include "util/StdIoMock.h"

using namespace ::util::test;

extern "C"
{
int32_t getByteFromStdin()
{
    auto& in          = StdIoMock::instance().in;
    int32_t const ret = in.size() != 0 ? in[0] : -1;
    in.advance(1);
    return ret;
}

void putByteToStdout(uint8_t b) { StdIoMock::instance().out.push_back(b); }
} /* extern "C" */
