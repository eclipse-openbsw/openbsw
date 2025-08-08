.. _names_hooking:

Names hooking
=============

The client of ``asyncFreeRtos`` module must provide implementation for the following functions,
that define certain macros in the configuration file **FreeRTOSConfig.h** of **FreeRTOS**:

* ``asyncEnterTask()``
* ``asyncLeaveTask()``
* ``asyncEnterIsrGroup()``
* ``asyncLeaveIsrGroup()``
* ``asyncTickHook()``
* ``asyncInitialized()``

Related types
-------------

* ``StaticContextHook``
