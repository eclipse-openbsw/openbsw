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

#include <util/logger/Logger.h>

// The purpose of the safety logger is to be able to log from the QM context, and for debugging
// during development. There should not be any logging from within the safety context.

DECLARE_LOGGER_COMPONENT(SAFETY)
