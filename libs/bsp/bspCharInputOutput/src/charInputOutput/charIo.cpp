#include "charInputOutput/charIo.h"

#include "charInputOutput/charIoSerial.h"

extern "C"
{
namespace
{
/**
 * Serial CharIO provided as default, can be replaced if necessary
 */
tCharIOApi charIOApi
    = {&SerialLogger_init,
       &SerialLogger_setAsynchronous,
       &SerialLogger_setSynchronous,
       &SerialLogger_putc,
       &SerialLogger_getc,
       &SerialLogger_Idle,
       &SerialLogger__outchar,
       &SerialLogger__inchar};
} // namespace

// below functions documented in the header file
void setCharIOApi(tCharIOApi const* const api) { charIOApi = *api; }

int setCharIOApi2Default()
{
    charIOApi.init            = &SerialLogger_init;
    charIOApi.setAsynchronous = &SerialLogger_setAsynchronous;
    charIOApi.setSynchronous  = &SerialLogger_setSynchronous;
    charIOApi.putc            = &SerialLogger_putc;
    charIOApi.getc            = &SerialLogger_getc;
    charIOApi.Idle            = &SerialLogger_Idle;
    charIOApi.__outchar       = &SerialLogger__outchar;
    charIOApi.__inchar        = &SerialLogger__inchar;
    charIOApi.init();
    return 1;
}

void charIoSetAsynchronous() { charIOApi.setAsynchronous(); }

void charIoSetSynchronous() { charIOApi.setSynchronous(); }

void charIoBackground() { charIOApi.Idle(); }

int charIoGetc() { return (charIOApi.getc()); }

int charIoPutc(int const c) { return (charIOApi.putc(c)); }

int charIO__inchar() { return (charIOApi.__inchar()); }

int charIO__outchar(int const c, int const last) { return (charIOApi.__outchar(c, last)); }
}
