// Copyright 2024 Accenture.

#ifndef GUARD_0CDC571F_CF9A_42B5_A547_265F29C8370D
#define GUARD_0CDC571F_CF9A_42B5_A547_265F29C8370D

class StubMock
{
protected:
    bool stub{true};

public:
    StubMock(bool val = true) : stub{val} {}

    void setStub(bool val) { stub = val; }

    bool isStub() const { return stub; }
};

#endif // GUARD_0CDC571F_CF9A_42B5_A547_265F29C8370D
