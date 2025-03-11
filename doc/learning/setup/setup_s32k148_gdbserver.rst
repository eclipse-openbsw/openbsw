.. _s32k148_gdb_via_usb:

Using GDB with the S32K148EVB board via USB
===========================================

The `S32K148 development board <https://www.nxp.com/design/design-center/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k148-q176-evaluation-board-for-automotive-general-purpose:S32K148EVB>`_
provides several ways to flash and debug images, one of which is via the USB / OpenSDA interface.
i.e. you can debug over a USB cable connected between the board and your computer.
This interface is implemented on the board by `P&E Micro <https://www.pemicro.com>`_
along with a matching Eclipse plugin which contains their implementation of gdb server.

P&E Micro's Eclipse plugin is included in NXP's IDE for the board (S32 Design Studio).
The instructions below set up the gdb server it contains to be used without S32 Design Studio.
This opens up the option of using your preferred IDE to debug on the board via USB.
For example see :ref:`setup_s32k148_gdb_vscode`.

Note that P&E Micro's implementation of gdb server must be used to debug the board via P&E Micro's implementation of the USB / OpenSDA interface.
P&E Micro provide three pre-built executables for three different host platforms - Linux, Windows and OS X.
The instructions on this page are focused on setting this up on Linux hosts, including on WSL (Windows Subsystem for Linux).

.. note::

   Please refer to :ref:`known_issues_with_s32k148_board`.

Get P&E Micro's gdb server from NXP's S32 Design Studio installation
--------------------------------------------------------------------

If you have already installed NXP's S32 Design Studio for the S32K148EVB board
then you already have P&E Micro's Eclipse Plug-in.
Check the subfolders of the S32 Design Studio installation for a folder named...

.. code-block:: bash

   eclipse/plugins/com.pemicro.debug.gdbjtag.pne_[version]/

which contains ...

.. code-block:: bash

   lin/pegdbserver_console
   win32/pegdbserver_console.exe
   osx/pegdbserver_console

These are P&E Micro's gdb server executables built for Linux, Windows and OS X.

Get P&E Micro's gdb server from their website
---------------------------------------------

If you have not installed NXP's S32 Design Studio,
(or if you want the latest version)
the `GDB Server Plug-In for Eclipse-based ARM IDEs <https://www.pemicro.com/products/product_viewDetails.cfm?product_id=15320151&productTab=1000000>`_
is free to download (you just need to `create an account <https://www.pemicro.com/login_new_user_form.cfm?mode=newUser>`_).

The downloaded file will be named ``com.pemicro.debug.gdbjtag.pne.updatesite-[VERSION].zip``.

Unzip it...

.. code-block:: bash

   unzip com.pemicro.debug.gdbjtag.pne.updatesite-[VERSION].zip

Once unzipped, in the ``plugins/`` folder, find the file named ``com.pemicro.debug.gdbjtag.pne_[VERSION].jar``.
You can unpack this using ``jar``. If you do not have ``jar``, install it...

.. code-block:: bash

   sudo apt install fastjar

Unpack the jar file...

.. code-block:: bash

   jar -xf com.pemicro.debug.gdbjtag.pne_[VERSION].jar

This produces the directory tree containing the three versions of ``pegdbserver_console``
for different host platforms, as shown above.
You will need to make ``pegdbserver_console`` executable for your host platform. eg.

.. code-block:: bash

   chmod a+x lin/pegdbserver_console

P&E Micro's OpenSDA USB Driver
------------------------------

P&E Micro's gdb server requires P&E Micro's OpenSDA USB driver to be installed.
If you have already installed NXP's S32 Design Studio then you should already have this driver installed.
If not, you can download it for free from `P&E Micro OpenSDA Support <https://www.pemicro.com/opensda/>`_.

Note that if you have installed NXP's S32 Design Studio on Windows but wish to set up gdb server in
WSL (Windows Subsystem for Linux) then you will need to attach the OpenSDA USB device to WSL
using ``usbipd`` as described in :ref:`setup_wsl_usb`
and install the Linux USB Driver for OpenSDA in WSL.

The downloaded OpenSDA driver for Linux will be in a file named ``pemicro-other-[VERSION].zip.tar.gz``.
Unpack this...

.. code-block:: bash

   tar -xzf pemicro-other-[VERSION].zip.tar.gz

and follow the instructions in ``pemicro-other-[VERSION]/drivers/PemicroLinuxDriversReadme.pdf``.
Note from `this issue <https://community.nxp.com/t5/S32K/OpenSDA-s32k144evb-Linux-Ubuntu-20-04-port-not-found/td-p/1050057>`_
that you may need to copy ``libp64-0.1.so.4`` to ``/usr/lib/`` if the installation script put it in ``/usr/lib64/``.
Also note from `this answer <https://askubuntu.com/a/1011218/1801515>`_,
on some systems installing ``libusb-dev`` satisfies the requirement to have the 32-bit version of ``libusb-0.1.so.4``.
On 64-bit Ubuntu :prop:`tool:ubuntu_version` the following successfully installs P&E Micro's OpenSDA USB driver...

.. code-block:: bash

   sudo apt-get install libusb-dev
   cd pemicro-other-20181128/drivers/libusb_64_32/libusb_64_32/
   sudo ./setup.sh

Running gdb server
------------------

Assuming you have the S32K148EVB board connected to you computer via USB and have set up your computer as described above,
you should be able to run the gdb server as follows...

.. code-block:: bash

   sudo ./pegdbserver_console -startserver -device=NXP_S32K1xx_S32K148F2M0M11

and if it successfully connects to you board you should see output like this...

.. code-block:: bash

   P&E GDB Server for Arm(R) devices, Version 9.45.00.00
   Copyright 2018, P&E Microcomputer Systems Inc, All rights reserved

   Loading library /home/username/temp/pemicro/com.pemicro.debug.gdbjtag.pne_5.7.8.202404031741/lin/gdi/unit_ngs_arm_internal.so ... Done.

   Command line arguments: -startserver -device=NXP_S32K1xx_S32K148F2M0M11
   Device selected is NXP_S32K1xx_S32K148F2M0M11
   HW Auto-Selected : Interface=OPENSDA Port=9988CE09   ; USB1 : OpenSDA (9988CE09)
   Connecting to target.
   OpenSDA detected - Flash Version 1.21
   Device is NXP_S32K1xx_S32K148F2M0M11.
   Mode is In-Circuit Debug.

   (C)opyright 2012, P&E Microcomputer Systems, Inc. (www.pemicro.com)
   API version is 101

   TARGET XML PATH is /home/username/temp/pemicro/com.pemicro.debug.gdbjtag.pne_5.7.8.202404031741/supportFiles_ARM/target_v7m_vfp.xml
   Server 1 running on 127.0.0.1:7224
   Server 2 running on 127.0.0.1:7226
   Server 3 running on 127.0.0.1:7228
   Server 4 running on 127.0.0.1:7230
   Server 5 running on 127.0.0.1:7232
   Server 6 running on 127.0.0.1:7234
   Server 7 running on 127.0.0.1:7236
   Server 8 running on 127.0.0.1:7238
   Server 9 running on 127.0.0.1:7240
   Server 10 running on 127.0.0.1:7242
   All Servers Running

Note that by default it uses localhost (``127.0.0.1``) and the ports listed (``7224``, ``7226``, ... ``7242``).
You can change this using the options ``-serverip`` and ``-serverport`` to make the gdb server available to other machines via ethernet.
Run ``pegdbserver_console -h`` to see the full list of options.

Running gdb
-----------

``arm-none-eabi-gdb`` is the gdb client for ARM.
If you have already set up your environment to build for the ``s32k148`` target
then you will have downloaded GCC for ARM toolchain and
added its ``bin`` directory to your ``PATH`` environment variable.
If so, then you should already have ``arm-none-eabi-gdb`` on your ``PATH``.

To run ``arm-none-eabi-gdb`` and connect to the board for a debugging session
check the following...

#. gdb server should be running as described above and successfully connecting to the board via USB

#. ``arm-none-eabi-gdb`` depends on the shared library ``libncurses.so.5``
   so you may need to install that...

   .. code-block:: bash

     sudo apt install libncurses5
#. You need to have built for the s32k148 target, such that the elf file is found at...

   .. code-block:: bash

     cmake-build-s32k148/application/app.referenceApp.elf

Then, in the root folder you can run the gdb client to start an interactive debug session...

  .. code-block:: bash

   arm-none-eabi-gdb -x tools/gdb/pegdbserver.gdb cmake-build-s32k148/application/app.referenceApp.elf

Flash the board on command-line
-------------------------------

With the gdb server running as described above,
instead of flashing the elf file via S32 Design Studio,
it may be more convenient to flash it in a single command as follows...

  .. code-block:: bash

   arm-none-eabi-gdb -batch -x test/pyTest/flash.gdb cmake-build-s32k148/application/app.referenceApp.elf

Reset the board on command-line
-------------------------------

You can reset the board in a single command as follows...

  .. code-block:: bash

   arm-none-eabi-gdb -batch -x test/pyTest/reset.gdb cmake-build-s32k148/application/app.referenceApp.elf
