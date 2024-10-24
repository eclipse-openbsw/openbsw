Simple direct way of using GDB on Ubuntu
========================================

This explains how to setup a gdb server which can be used with all kinds of GDB enabled clients
including the gdb command line client.

Requirements: gdb-server, arm-none-eabi-gdb, libncurses5

The S32K148EVB board comes with OpenSDA which can be accessed via the USB port. The J8 and J18
jumpers have to be in the 2-3 position if the power is supplied only from the USB port
(https://www.mouser.com/datasheet/2/302/S32K148EVB-QSG-1358529.pdf).
Current boards come with OpenSDA v1 which can be accessed only by the pemicro gdb server
(pegdbserver_console). The pegdbserver_console is integrated in an Eclipse plugin.

Installation steps
------------------

1. Download the Eclipse plugin:

   Register for a free account at PEmicro in order to download the plugin from
   https://www.pemicro.com/products/product_viewDetails.cfm?product_id=15320151&productTab=1000000

2. Unzip the archive(In a new folder)


3. Unzip plugin jar, for example ``plugins/com.pemicro.debug.gdbjtag.pne_5.7.8.202404031741.jar``


4. Use pegdbserver_console file:

The pegdbserver_console file is located in the ``lin`` folder.
(Note: Windows users can also use this method, choosing the "win" server here and slightly adapting
the instructions below)

5. Board connection:

Connect the board to your computer via USB.

6. Run the pegdbserver_console:

.. code-block:: bash

   sudo ./pegdbserver_console -startserver -device=NXP_S32K1xx_S32K148F2M0M11 -serverport=7224

7. Run gdb in workspace root:

.. code-block:: bash

   arm-none-eabi-gdb -x tools/gdb/pegdbserver.gdb cmake-build-s32k148/application/app.referenceApp.elf

arm-none-eabi-gdb is in the same location where arm-none-eabi-gcc is.

