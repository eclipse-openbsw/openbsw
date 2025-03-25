// Copyright 2024 Accenture.

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

