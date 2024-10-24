asyncConsole - Asynchronous Console
===================================

This library provides an extension for operating the BSW serial console in an asynchronous
manner. Typically the keyboard input is fetched and synchronously processed in the system's idle
or background task. The classes provided by this library can be used to execute the inputted
command in another task context (to avoid critical sections, this shall be the context of the
function, triggered by the command).