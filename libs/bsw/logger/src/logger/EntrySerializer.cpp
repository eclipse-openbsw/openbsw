// Copyright 2024 Accenture.

#include "logger/EntrySerializer.h"

namespace logger
{
SectionPredicate::SectionPredicate(void const* const sectionStart, void const* const sectionEnd)
: _sectionStart(sectionStart), _sectionEnd(sectionEnd)
{}

} // namespace logger
