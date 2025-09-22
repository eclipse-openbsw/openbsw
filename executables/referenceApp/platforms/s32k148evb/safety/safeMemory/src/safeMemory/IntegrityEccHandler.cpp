// Copyright 2025 Accenture.

#include "safeMemory/IntegrityEccHandler.h"

#include <mcu/mcu.h>

namespace safety
{
namespace safe_memory
{

bool checkRamDoubleBitError()
{
    if (0U != (ERM->SR0 & (ERM_SR0_NCE0_MASK | ERM_SR0_NCE1_MASK)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool checkFlashDoubleBitError()
{
    if (0U != (FTFC->FERSTAT & FTFC_FERSTAT_DFDIF_MASK))
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t readErmMemoryErrorAddress()
{
    return ERM->EARn[0].EAR != 0 ? ERM->EARn[0].EAR : ERM->EARn[1].EAR;
}

} // namespace safe_memory
} // namespace safety
