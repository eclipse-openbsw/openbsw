.. _learning_setup:

Set up your environment
=======================

In order to begin learning about the software in this project,
a good first step is to set up an environment in which you can build the code and then execute the built image.
Example setups are described for the following types of host machine...

.. toctree::
   :maxdepth: 1
   :glob:

   setup_env_*

If your preferred IDE is Visual Studio Code then see...

.. toctree::
   :maxdepth: 1
   :glob:

   setup_vscode

Using the S32K148 Development Board
-----------------------------------

If you have access to a
`S32K148 development board <https://www.nxp.com/design/design-center/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k148-q176-evaluation-board-for-automotive-general-purpose:S32K148EVB>`_
then below is some important information to note...

.. _known_issues_with_s32k148_board:

Known Issues with the S32K148 board
+++++++++++++++++++++++++++++++++++

.. list-table::
    :header-rows: 1
    :width: 100%
    :widths: 10 90

    * - #
      - Description
    * - 1
      - The board can be bricked if the wrong values are flashed to the Flash Configuration Field
        so it is best not to change anything in the linker script ``executables/referenceApp/platforms/s32k148evb/main/linkerscript/application.dld``
        unless you are very confident about your changes.
    * - 2
      - While the board can be powered from USB alone,
        it has been reported that CAN and Ethernet do not work
        unless an external power supply (12-volt, 5.5 mm barrel connector) is used.
    * - 3
      - While NXP provide a linux installer for S32 Design Studio for S32 Platform, according to the installation manual
        the most recent version of Ubuntu listed as supported is Ubuntu :prop:`tool:ubuntu_version`.
        S32 Design Studio for S32 Platform has been successfully installed in Ubuntu :prop:`tool:ubuntu_version` as documented in :doc:`setup_s32k148_ubuntu_nxpide`
        but please note that many people have encountered difficulties with this, especially at the step to enable it
        using an activation code. If you have difficulties you may prefer to set up ``gdbserver`` as described in :doc:`setup_s32k148_gdbserver`
        with an IDE of your choice.
    * - 4
      - P&E Micro provide versions of gdb server for Windows, Linux and Mac that work with their OpenSDA USB drivers
        to support flashing and debugging on the board via USB. See :ref:`s32k148_gdb_via_usb`.
        However the USB drivers are only available for Windows and Linux on x86 based hosts, not for ARM,
        and so debugging on the board from ARM hosts (eg. a Raspberry Pi or an ARM-based Mac) requires an external debugger,
        such as a Lauterbach, working via JTAG.

Jumper setup on the S32K148 board
+++++++++++++++++++++++++++++++++

Before connecting power to the target board, check the jumpers on the target board are all in their default positions as shown below...

   .. image:: nxp_board_top_jumpers.png
      :width: 80%
      :align: center

USB connection from host to board
+++++++++++++++++++++++++++++++++

The USB connection on the board provides both a serial console and a debugger connection for flashing images and debugging on the board.
Connect the USB cable to a PC and connect micro USB connector of the USB cable to micro-B port **J24** on the S32K148EVB.

CAN connection on the S32K148 board
+++++++++++++++++++++++++++++++++++

If you are using a CAN adapter to communicate from your PC to the board, connect it to the **J11** on the board as shown below...

   .. image:: nxp_board_can_plug.png
      :width: 80%
      :align: center

For further information, see the links on this page: :ref:`s32k148evb_overview`.

Next: :ref:`learning_layout`
