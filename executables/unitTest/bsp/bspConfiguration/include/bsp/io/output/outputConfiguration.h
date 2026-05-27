/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/


#if (BSPOUTPUTCONFIG == 1)

Output::OutputConfig const Output::sfOutputConfigurations[][Output::NUMBER_OF_INTERNAL_OUTPUTS] = {
    {
        /* 00 */ {Io::TestPin0, Io::HIGH, Io::HIGH_ACTIVE},
    },
};

Output::OutputConfig const* Output::getConfiguration(uint8_t /* hw */)
{
    return &sfOutputConfigurations[0][0];
}

#else

enum OutputId
{
    /* 00 */ MOCK_OUTPUT_1,
    NUMBER_OF_INTERNAL_OUTPUTS,
    NUMBER_OF_EXTERNAL_OUTPUTS = NUMBER_OF_INTERNAL_OUTPUTS - 1,
    TOTAL_NUMBER_OF_OUTPUTS,
};

#endif /* BSPOUTPUTCONFIG == 1 */
