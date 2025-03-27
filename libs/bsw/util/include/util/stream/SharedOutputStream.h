// Copyright 2024 Accenture.

#pragma once

#include "util/stream/ISharedOutputStream.h"

namespace util
{
namespace stream
{
/**
 * The ISharedOutputStream class implementation.
 *
 */
class SharedOutputStream : public ISharedOutputStream
{
public:
    explicit SharedOutputStream(IOutputStream& strm);

    IOutputStream& startOutput(IContinuousUser* user) override;
    void endOutput(IContinuousUser* user) override;
    void releaseContinuousUser(IContinuousUser& user) override;

private:
    IOutputStream& _stream;
    IContinuousUser* _user;
};

} // namespace stream
} // namespace util
