.. _learning_can:

Working with CAN
================

Previous: :ref:`learning_commands`

The POSIX and S32K148 builds support CAN but in different ways.
The S32K148 development board has CAN hardware and this is used to send and receive CAN frames
whereas on the POSIX platform a CAN socket is used to send CAN frames to a Virtual CAN interface
(note this requires that the POSIX platform used includes ``SocketCan`` support).

POSIX build
-----------

Assuming you have successfully built and run the executable ``app.referenceApp.elf`` on a POSIX platform
as instructed in :ref:`setup_posix_ubuntu_build`
then you may see one of the following errors during startup of ``app.referenceApp.elf``...

.. code-block:: bash

    1617576: RefApp: CAN: ERROR: [SocketCanTransceiver] Failed to create socket (node=vcan0, error=-1)

or

.. code-block:: bash

    1617576: RefApp: CAN: ERROR: [SocketCanTransceiver] Failed to ioctl socket (node=vcan0, error=-1)

which indicate that a socket could not be opened using the Virtual CAN interface ``vcan0`` -
you need to set it up. On Ubuntu :prop:`tool:ubuntu_version`, the following commands will set it up...

.. code-block:: bash

    sudo ip link add dev vcan0 type vcan
    sudo ip link set vcan0 mtu 16
    sudo ip link set up vcan0

Note that the above commands are also in the script ``tools/can/bring-up-vcan0.sh``.

If you see an error ``Error: Unknown device type.``
from the above commands then you need to figure out how to set up ``SocketCan`` support
and support for device type ``vcan`` on your platform.

.. note::

    Please note while the standard Ubuntu :prop:`tool:ubuntu_version` release includes ``SocketCan`` support,
    the version of Ubuntu :prop:`tool:ubuntu_version` available for WSL does not
    - to perform the steps in this exercise on WSL, a custom kernel would need to be created with ``SocketCan`` support.
    See :ref:`setup_wsl_socketcan`.

Once ``vcan0`` is set up the above error messages should no longer be seen during the application's initialization.

In ``DemoSystem.cpp``, the software sends a CAN frame every second with Frame ID = ``0x558``
and with 4 bytes of data containing a counter.
If initialization completes successfully,
you should see a log message every second indicating a CAN frame was sent...

.. code-block:: bash

    2477109: RefApp: CAN: INFO: [CanDemoListener] CAN frame sent, id=0x558, length=4

You can check that this is sent to ``vcan0`` using the utility ``candump``
which can be installed on Ubuntu :prop:`tool:ubuntu_version` as follows...

.. code-block:: bash

    sudo apt install can-utils

While ``app.referenceApp.elf`` is running, in another shell terminal run...

.. code-block:: bash

    candump vcan0

and you should see the CAN frames being received every second and the counter values incrementing, like this...

.. code-block:: bash

    vcan0  558   [4]  00 00 00 00
    vcan0  558   [4]  00 00 00 01
    vcan0  558   [4]  00 00 00 02
    vcan0  558   [4]  00 00 00 03
    vcan0  558   [4]  00 00 00 04

Using ``cansend`` (also installed with ``can-utils``) you can send a CAN frame to ``vcan0``.
With ``app.referenceApp.elf`` running in one shell terminal and ``candump vcan0`` running in another shell terminal,
open a third shell terminal and send a CAN frame as follows...

.. code-block:: bash

    cansend vcan0 321#AABBCCDDEEFF

The above command sends a CAN Frame with ID = ``0x321`` and 6 bytes of data ``0xAABBCCDDEEFF`` to ``vcan0``.
You should see it in the output from ``candump vcan0`` like this...

.. code-block:: bash

    vcan0  321   [6]  AA BB CC DD EE FF

and you should see it being received by the application in its log like this...

.. code-block:: bash

    14658083: RefApp: CAN: DEBUG: [SocketCanTransceiver] received CAN frame, id=0x321, length=6

Next, take a look at the code in ``referenceApp/executables/referenceApp/application/src/app/CanDemoListener.cpp``.
You should see this code in ``CanDemoListener::init()``...

.. code-block:: cpp

    _canFilter.add(0x123);
    _canFilter.add(0x124);

This sets up ``CanDemoListener`` as a listener for CAN frames with these IDs.
If these are received then ``CanDemoListener::frameReceived`` will be called, which contains...

.. code-block:: cpp

    void CanDemoListener::frameReceived(::can::CANFrame const& frame)
    {
        Logger::info(
            CAN,
            "[CanDemoListener] received CAN frame, id=0x%X, length=%d",
            (int)frame.getId(),
            (int)frame.getPayloadLength());

        if (_canTransceiver != nullptr)
        {
            ::can::CANFrame newFrame(frame.getId() + 1, frame.getPayload(), frame.getPayloadLength());
            auto result = _canTransceiver->write(newFrame);
            Logger::info(CAN, "[CanDemoListener] response queued, result = %d", (int)result);
        }
    }

This code means that if a CAN Frame with ID ``0x123`` or ``0x124`` is received,
then a new frame is created with the same data and its ID incremented by 1
and is transmitted by the CAN transceiver.

Let's test that.
With ``app.referenceApp.elf`` running in one shell terminal, ``candump vcan0`` running in another shell terminal,
in a third shell terminal send a CAN frame as follows...

.. code-block:: bash

    cansend vcan0 124#88776655

In the application's log output you should see...

.. code-block:: bash

    16040039: RefApp: CAN: DEBUG: [SocketCanTransceiver] received CAN frame, id=0x124, length=4
    16040039: RefApp: CAN: INFO: [CanDemoListener] received CAN frame, id=0x124, length=4
    16040039: RefApp: CAN: INFO: [CanDemoListener] response queued, result = 0
    16040041: RefApp: CAN: INFO: [CanDemoListener] CAN frame sent, id=0x125, length=4

and in the output from ``candump vcan0`` you should see...

.. code-block:: bash

    vcan0  124   [4]  88 77 66 55
    vcan0  125   [4]  88 77 66 55

which shows the frame with ID ``0x124`` sent by ``cansend``
and the response frame with ID ``0x125`` sent by the application.

S32K148 build
-------------

To prove is CAN working in a S32K148 build
you will need CAN hardware that connects to the CAN interface on the S32K148 development board's J11 connector.
Any device that supports Classic CAN (CAN-FD is not needed) should work if its CAN parameters are set up as follows...

.. list-table::
    :header-rows: 1
    :stub-columns: 1
    :widths: auto

    * - Bitrate
      - Sample point
    * - 500kbits/s
      - 88%

The same ``DemoSystem`` and ``CanDemoListener`` classes are used in the S32K148 and POSIX builds
(note different ``CanSystem`` are used)
so you should be able to see the same CAN behaviour on the S32K148
as described above for the POSIX build.

Find the ``DemoSystem``, ``CanDemoListener`` and ``CanSystem`` classes and examine how these work.
In particular note that for the POSIX build, ``CanSystem`` is implemented using ``::can::SocketCanTransceiver``
whereas for the S32K148 build, ``CanSystem`` is implemented using ``bios::CanFlex2Transceiver``
both of which are alternative implementations of ``::can::AbstractCANTransceiver``.

Next: :ref:`learning_uds`
