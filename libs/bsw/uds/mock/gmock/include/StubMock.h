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

class StubMock
{
protected:
    bool stub{true};

public:
    StubMock(bool val = true) : stub{val} {}

    void setStub(bool val) { stub = val; }

    bool isStub() const { return stub; }
};
