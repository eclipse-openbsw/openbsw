.. _setup_wsl_usb:

Add USB support to WSL
======================

Using `usbipd-win <https://github.com/dorssel/usbipd-win/>`__
you can make USB devices that are connected to your Windows PC/laptop available in WSL.

As an example, the steps below were used to make a Peak USB-CAN adaptor accessible as a network interface in Ubuntu using ``SocketCAN``.
This assumes you have already successfully followed the steps in :ref:`setup_wsl_socketcan` and your WSL kernel supports USB.
Recent versions of Windows running WSL kernel :prop:`tool:wsl_kernel_version` or later already include support for many USB scenarios.

The steps below were performed with WSL up and running - with the CAN kernel modules loaded
and ``vcan0`` successfully created as described in :ref:`setup_wsl_socketcan`.

First, download the latest `usbipd-win <https://github.com/dorssel/usbipd-win/>`__ Windows installer and run it (you must have Administrator rights).

Once installed, in a command prompt with Administrator rights run ``usbipd list``

.. code-block:: bash

    C:\>usbipd list
    Connected:
    BUSID  VID:PID    DEVICE                                                        STATE
    2-3    0c45:6a09  Integrated Webcam                                             Not shared
    2-13   1357:0089  OpenSDA - CDC Serial Port (http://www.pemicro.com/opensda...  Not shared
    2-14   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared
    3-2    0c72:000c  PCAN-USB                                                      Not shared
    3-3    047f:c025  Plantronics C320-M, USB Input Device                          Not shared
    4-3    0b95:1790  ASIX AX88179 USB 3.0 to Gigabit Ethernet Adapter              Not shared

You can see in the above list the Peak CAN USB adaptor is listed as ``PCAN-USB`` with Bus ID = ``3-2``

In the same command prompt with Administrator rights run ``usbipd bind --busid 3-2``.

Now, when the devices are listed again you can see that the Peak CAN USB adaptor is "Shared",
meaning the device is available to WSL.

.. code-block:: bash

    C:\>usbipd list
    Connected:
    BUSID  VID:PID    DEVICE                                                        STATE
    2-3    0c45:6a09  Integrated Webcam                                             Not shared
    2-13   1357:0089  OpenSDA - CDC Serial Port (http://www.pemicro.com/opensda...  Not shared
    2-14   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared
    3-2    0c72:000c  PCAN-USB                                                      Shared
    3-3    047f:c025  Plantronics C320-M, USB Input Device                          Not shared
    4-3    0b95:1790  ASIX AX88179 USB 3.0 to Gigabit Ethernet Adapter              Not shared

This "Shared" status persists across reboots so you will not need to do this again
and you won't need Administrator rights for the remaining commands.

Once it is "Shared", a device can be "Attached" to WSL in a normal Windows command prompt (without Administrator rights) as follows...

.. code-block:: bash

    C:\>usbipd attach --wsl --busid 3-2
    usbipd: info: Using WSL distribution 'Ubuntu-x.x' to attach; the device will be available in all WSL 2 distributions.
    usbipd: info: Using IP address 127.0.0.1 to reach the host.

The "Attached" status does not persist, you will need to reattach after every time WSL is restarted.
While "Attached" to WSL, you cannot use the device in Windows.

.. code-block:: bash

    C:\>usbipd list
    Connected:
    BUSID  VID:PID    DEVICE                                                        STATE
    2-3    0c45:6a09  Integrated Webcam                                             Not shared
    2-13   1357:0089  OpenSDA - CDC Serial Port (http://www.pemicro.com/opensda...  Not shared
    2-14   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared
    3-2    0c72:000c  PCAN-USB                                                      Attached
    3-3    047f:c025  Plantronics C320-M, USB Input Device                          Not shared
    4-3    0b95:1790  ASIX AX88179 USB 3.0 to Gigabit Ethernet Adapter              Not shared

Now, in a Ubuntu shell, you should see the USB device...

.. code-block:: bash

    frank@C11-JQXQXCRB1VG:~$ lsusb
    Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
    Bus 001 Device 002: ID 0c72:000c PEAK System PCAN-USB
    Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    frank@C11-JQXQXCRB1VG:~$

Next, create the file ``/etc/systemd/network/80-can.network`` containing...

.. code-block:: bash

    [Match]
    Name=can*

    [CAN]
    BitRate=500000

Then enable and start ``systemd-networkd``...

.. code-block:: bash

    sudo systemctl enable systemd-networkd
    sudo systemctl start systemd-networkd

If this is successful, a new network interface called ``can0`` should be created.

.. code-block:: bash

    frank@C11-JQXQXCRB1VG:~$ ifconfig
    can0: flags=193<UP,RUNNING,NOARP>  mtu 16
            unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
            RX packets 23  bytes 184 (184.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

After reboots, once the ``PCAN-USB`` is again "Attached" in a Windows command prompt,
and once the CAN modules are loaded in Ubuntu,
then the same ``can0`` interface will be recreated.

You can use the utilities ``candump`` and ``cansend`` with ``can0``, just as you did with ``vcan0`` in :ref:`setup_wsl_socketcan`
but in this case the CAN messages sent/received should be using the Peak CAN USB adaptor.
If you have a S32K148 Development Board attached to the Peak CAN USB adaptor
then using ``candump`` you should see CAN messages sent from an image running on that board.

Also, you can change the POSIX platform's code so that it attaches to ``can0`` instead of ``vcan0``,
then the POSIX build can communicate with the S32K148 Development Board over the Peak CAN USB adaptor.
