// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void disableAllInterrupts(void);
void enableAllInterrupts(void);

bool areInterruptsDisabled(void);
bool areInterruptsEnabled(void);

#ifdef __cplusplus
}
#endif

