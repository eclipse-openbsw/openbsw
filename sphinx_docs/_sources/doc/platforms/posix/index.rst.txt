.. _posix_overview:

POSIX
=====

Overview
--------
- Eclipse OpenBSW is intended for automotive embedded platforms, it can be built and run on
  POSIX platforms without automotive specific hardware.
- For example, where an embedded platform typically provides a serial console via a UART device,
  on the POSIX platform the same console is available directly in a terminal interface. While CAN
  is widely used in automotive networks using CAN specific hardware, if the POSIX platform
  supports SocketCan then CAN applications can be implemented using this, bypassing the need for
  CAN hardware.
- The purpose of supporting POSIX platforms is to accelerate learning and support widespread adoption
  of Eclipse OpenBSW.

Build environment
-----------------
For instructions on building for this platform see :ref:`setup_posix_ubuntu_build`.
