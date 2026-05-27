/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
