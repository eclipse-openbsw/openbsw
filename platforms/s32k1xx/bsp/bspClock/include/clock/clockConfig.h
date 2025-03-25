// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void configurPll();
void fircOff();
void extOscOff();
void pllOff();
void systemClock2Sirc();
void systemClock2Firc();
void systemClock2Pll();

#ifdef __cplusplus
}
#endif

