.. _learning_ethernet:

Working with Ethernet
=====================

Both the POSIX and S32K148 builds support Ethernet functionality, but they do so using different
configurations.

Ethernet Setup:
   - S32K148 uses a development board in combination with the TJA1101 Automotive Ethernet PHY
     transceiver daughter board. This provides a physical Ethernet interface suitable for
     automotive applications.
   - POSIX employs a TAP (Network TUN/TAP) Ethernet module, which provides a virtual Ethernet
     interface for simulation and testing.

Refer :ref:`application_ethernetSystem` for more details on the Ethernet application system.

Set-up Environment
------------------

Before proceeding with the Ethernet setup, ensure you have built the openBsw project for the
respective platform.

Reference for instructions:
   - For POSIX Build: :ref:`Ubuntu<setup_posix_ubuntu_build>` or :ref:`Windows<setup_posix_win_build>`.
   - For S32K148 Build: :ref:`Ubuntu<setup_s32k148_ubuntu_build>` or
     :ref:`Windows<setup_s32k148_win_build>`.

POSIX
+++++

A ``tap0`` interface is needed for virtual Ethernet. The `bring-up-ethernet.sh` script will set it up.
While running this script in the terminal, you need sudo permission.

Setup in Testing PC:
   - Navigate to the openBsw root directory.
   - Run the script to set up the tap interface: ``./tools/enet/bring-up-ethernet.sh``
   - Run the application on POSIX with the below command in the terminal:
     ``cmake-build-posix/application/app.referenceApp.elf``

S32K148
+++++++

Hardware Connection:
   - Connect the S32K148 development board to the TJA1101 daughter board.

      * Ensure the TJA1101 board is configured as master mode (Remove the jumper).

   - You need a media converter (automotive Ethernet to standard Ethernet) to test on a PC,
     since TJA1101 has automotive Ethernet as output.

      * Connect the TJA1101 daughter board (100Base-T1) to the media converter using a twisted pair cable.
      * Connect the media converter to the PC using a standard Ethernet cable.
      * If master/slave mode is available in the media converter, ensure it is configured as slave mode.
      * If an external power supply is required for the media converter, 12v is available from the
        S32K148 development board itself (J3-3 pin).

      .. image:: tja1101.png
         :width: 40%
         :align: center

Setup in Testing PC:
   - Set the ``IP`` of the Ethernet port in the testing PC to ``192.168.0.1`` (Static IP)

      * In Windows, changing IP assignment to manual can be done from: ``Control Panel ->
        Network and Internet -> Ethernet``.
      * IPv4 subnet mask can be configured as ``255.255.255.0``

   - Flash the application to the S32K148 board as instructed in
     :ref:`Using NXP S32DS<setup_s32k148_win_nxpide>` or :ref:`Using GDB Debugger<s32k148_gdb_via_usb>`.

.. note::
   If the ``tap0`` interface added for POSIX is still active, it needs to be removed
   before testing on the S32K148. You can use the below command in the terminal:
   ``sudo ip tuntap del dev tap0 mode tap``

Ping Test
---------

To verify the Ethernet connection, you can perform a ping test.

Open a terminal and run:

- For POSIX: ``ping 192.168.0.200``
- For S32K148: ``ping 192.168.0.201``

This should show a response indicating that the Ethernet connection is working correctly.

.. note::
   The static IP configured in the `netif` module for the POSIX platform is ``192.168.0.201`` and for S32K148
   ``192.168.0.200``.

Test with ncat/telnet
---------------------
To test the Ethernet functionality, you can use `ncat` or `telnet` for simple TCP/UDP communication.
Since our demo application has echo server functionality implemented in `EthernetSystems`, you
can send messages and receive them back as an echo.

Ensure you have `ncat` and `telnet` installed on your system.

- UDP:
    * POSIX: ``ncat -u 192.168.0.201 49444``
    * S32K148: ``ncat -u 192.168.0.200 49444``

  Once the connection is established, type your message and press ``enter``, the echo will
  be printed back. To exit from ncat: ``ctrl + C``.

- TCP:
    * POSIX: ``telnet 192.168.0.201 49555``
    * S32K148: ``telnet 192.168.0.200 49555``

  Once the connection is established, type your message and press ``enter``, the echo will
  be printed back. To exit from telnet: ``ctrl + ]`` and type ``quit``.

Test with Iperf
---------------
Iperf is a tool for measuring the maximum TCP and UDP bandwidth performance. We have implemented
iperf basic server functionality in the `EthernetSystems` application, allowing you to test the
network performance using the iperf client.

.. note::
   Ensure that the `iperf2` tool is installed on your system before running the commands below.
   In Ubuntu, you can install it using: ``sudo apt install iperf``. We tested with `version 2.1.5`.

- Test Ethernet on POSIX:
    * TCP: ``iperf -c 192.168.0.201 -w 2739 -l 1459 -t 5``
    * UDP: ``iperf -c 192.168.0.201 -u -l 1000 -b 10M -t 5``
- Test Ethernet on S32K148:
    * TCP: ``iperf -c 192.168.0.200 -w 2739 -l 1459 -t 5``
    * UDP: ``iperf -c 192.168.0.200 -u -l 1000 -b 10M -t 5``

Test with Pytest
----------------
Functional testing of the Ethernet application is done using Pytest.
The Pytest script is placed under ``test/pyTest/enet/test_ethernet.py``.
Refer to ``test/pyTest/README.md`` for instructions on how to run the Pytest script.
