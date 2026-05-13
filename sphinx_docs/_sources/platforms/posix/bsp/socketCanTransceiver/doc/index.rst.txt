socketCanTransceiver
====================

Overview
--------

This module implements ``ICanTransceiver`` interface using Linux SocketCAN stack.

The module communicates over an existing Linux CAN network interface, which can designate
a physical or a virtual CAN device. The particular network interface name to use is passed
as a configuration parameter.

Architecture
------------

The ``SocketCanTransceiver`` object opens a POSIX socket in the non-blocking mode and periodically
tries to send and receive CAN frames through it. The frames to be sent are queued in the
``::io::MemoryQueue<>``, where they are put by the ``write()`` method and stored as memory copies
of ``CAHFrame`` objects. The code makes basic checks at compile time that the ``CANFrame`` object
can safely be duplicated using ``std::memcpy()``. The queueing also makes sure that the
``IFilteredCANFrameSentListener`` callbacks will be called in the proper task context.


Integration
-----------

The transceiver static configuration (a ``SocketCanTransceiver::DeviceConfig`` object) is passed
to the ``SocketCanTransceiver`` constructor by reference. The configuration object shall be valid
during the lifetime on the ``SocketCanTransceiver`` object.

All the methods of the ``SocketCanTransceiver`` object, except for the constructor, the destructor,
and the ``write()`` method, shall be called in the same task context (normally the CAN task
context), to avoid race conditions in state changes. The listener callbacks registered via the
``ICanTransceiver`` interface will be called in the same task context. Calling the ``write()``
method from the listener callbacks is safe.

The method ``run()`` needs to be periodically called in order to trigger the actual sending and
receiving of CAN frames.
