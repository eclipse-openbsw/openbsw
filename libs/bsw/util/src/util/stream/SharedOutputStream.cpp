/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/SharedOutputStream.h"

namespace util
{
namespace stream
{
SharedOutputStream::SharedOutputStream(IOutputStream& strm)
: ISharedOutputStream(), _stream(strm), _user(nullptr)
{}

IOutputStream& SharedOutputStream::startOutput(IContinuousUser* const user)
{
    if ((_user != nullptr) && (user != _user))
    {
        _user->endContinuousOutput(_stream);
    }
    _user = user;
    return _stream;
}

void SharedOutputStream::endOutput(IContinuousUser* const user) { _user = user; }

void SharedOutputStream::releaseContinuousUser(IContinuousUser& user)
{
    if (_user == &user)
    {
        _user = nullptr;
        user.endContinuousOutput(_stream);
    }
}

} // namespace stream
} // namespace util
