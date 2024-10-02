// Copyright 2024 Accenture.

#ifndef GUARD_191C6119_8432_4C4B_B751_78CD11E4FA3A
#define GUARD_191C6119_8432_4C4B_B751_78CD11E4FA3A

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

#endif /* GUARD_191C6119_8432_4C4B_B751_78CD11E4FA3A */
