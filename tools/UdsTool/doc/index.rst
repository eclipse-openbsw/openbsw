.. _UdsTool:

UdsTool
=======

Overview
--------

A simple tool to talk to ECUs via UDS (ISO 14229-1).

Installation
------------
Inside the ``tools/UdsTool`` folder run: ``pip install .``

.. note::

    - If you are using WSL, make sure you have already successfully followed the steps in :ref:`setup_wsl_socketcan` and your WSL kernel supports USB.
      Recent versions of Windows running WSL kernel :prop:`tool:wsl_kernel_version` or later already include support for many USB scenarios.
    - If you would like to be able to edit the source code, you can install it in editable mode by adding the ``editable`` flag in the command: ``pip install --editable .``

Usage
-----
You can view all the options available by executing the following command:

    .. code-block::

      > udstool --help

      Usage: udstool [OPTIONS] COMMAND [ARGS]...

      Options:
        --help  Show this message and exit.

      Commands:
        raw       Enter raw command to send
        read      UDS service Read Data By Identifier (0x22)
        routine   UDS service Routine Control (0x31)
        security  UDS service Security Access (0x27)
        session   UDS service Diagnostic Session Control (0x10)
        write     UDS service Write Data By Identifier (0x2e)

Services Supported
------------------
.. list-table::
    :header-rows: 1
    :stub-columns: 1
    :widths: auto

    * - **Service**
      - **SID**
    * - Read Data By Identifier
      - ``0x22``
    * - Write Data By Identifier
      - ``0x2E``
    * - Session Control
      - ``0x10``
    * - ECU Reset
      - ``0x11``
    * - Security Access
      - ``0x27``
    * - Routine Control
      - ``0x31``
    * - Request Download
      - ``0x34``
    * - Transfer Data
      - ``0x36``
    * - Transfer Exit
      - ``0x37``

All these services can be requested using the ``raw`` command. For example :
  ``udstool raw --eth --host [Host IP] --ecu [ECU logical address] --source [Client logical address] --data [UDS payload ex:22cf01]``

Example
-------
- In POSIX environment:

    To send a Read Data By Identifier (RDBI) request for DID ``0xCF01``:

    ``udstool read --can --channel vcan0 --txid [TxId] --rxid [RxId] --did cf01 --config [Path to config file]``

- In S32K1xx environment:

    ``udstool read --can --channel pcan --txid [TxId] --rxid [RxId] --did cf01 --config [Path to config file]``

.. note::
    + You can find a reference canConfig.json file in ``tools/UdsTool/app/canConfig.json``.
    + In the demo application the `TxId` and `RxId` are set to ``0x002A`` and ``0x00F0`` respectively.
