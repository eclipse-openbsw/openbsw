asyncConsole - Asynchronous Console
===================================

This library provides an `AsyncConsole` class which allows for the registration of :ref:`bsw commands <util_command>` with the help of wrapper-classes.

Typically the keyboard input is fetched and synchronously processed in the system's idle
or background task. The classes provided by this library can be used to execute the entered
command in a different task context (to avoid critical sections, this shall be the context of the
function, triggered by the command).

.. toctree::
   :glob:

   */index
