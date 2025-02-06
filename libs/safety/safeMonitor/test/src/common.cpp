// Copyright 2025 Accenture.

#include "common.h"

int ScopedMutexMock::_numConstructed = 0;
int ScopedMutexMock::_numDestructed  = 0;

ScopedMutexMock::ScopedMutexMock() { _numConstructed++; }

ScopedMutexMock::~ScopedMutexMock() { _numDestructed++; }

void ScopedMutexMock::reset()
{
    _numConstructed = 0;
    _numDestructed  = 0;
}

int ScopedMutexMock::numConstructed() { return _numConstructed; }

bool ScopedMutexMock::allDestructed() { return _numConstructed == _numDestructed; }
