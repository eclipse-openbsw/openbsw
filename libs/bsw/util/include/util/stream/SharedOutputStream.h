/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
