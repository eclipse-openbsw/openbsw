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

namespace bsp
{
/**
 * Return code for all bsp functions
 */
enum BspReturnCode
{
    /** everything OK */
    BSP_OK,
    /** something failed */
    BSP_ERROR,
    /** not supported */
    BSP_NOT_SUPPORTED
};

} /* namespace bsp */
