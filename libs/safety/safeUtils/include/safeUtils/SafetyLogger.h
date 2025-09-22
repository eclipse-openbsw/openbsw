// Copyright 2024 Accenture.

#pragma once

#include <util/logger/Logger.h>

// The purpose of the safety logger is to be able to log from the QM context, and for debugging
// during development. There should not be any logging from within the safety context.

DECLARE_LOGGER_COMPONENT(SAFETY)
