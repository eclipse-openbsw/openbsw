/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "logger/EntrySerializer.h"

namespace logger
{
SectionPredicate::SectionPredicate(void const* const sectionStart, void const* const sectionEnd)
: _sectionStart(sectionStart), _sectionEnd(sectionEnd)
{}

} // namespace logger
