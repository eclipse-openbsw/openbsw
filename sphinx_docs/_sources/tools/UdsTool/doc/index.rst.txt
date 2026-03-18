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

Examples
--------

UDS on CAN
++++++++++

To send UDS requests over CAN to the demo application running on POSIX, you can use the following commands:

- To send a Read Data By Identifier (RDBI) request for DID ``0xCF01``:

  ``udstool read --can --channel [CAN interface] --txid [ECU logical address] --rxid [Client logical address] --did cf01 --config [Path to config file]``

- To send a Write Data By Identifier (WDBI) request for DID ``0xCF03`` with data ``999``:

  ``udstool write --can --channel [CAN interface] --txid [ECU logical address] --rxid [Client logical address] --did cf03 --data 999 --config [Path to config file]``

- To send a Diagnostic Session Control request to switch to session id ``0x01`` (Default session):

  ``udstool session --can --channel [CAN interface] --txid [ECU logical address] --rxid [Client logical address] --id 01 --config [Path to config file]``

.. note::
    + For POSIX target, make sure to set up a virtual CAN interface vcan0.
    + For S32K1xx target, make sure the CAN interface is set up and passed as ``--channel`` parameter to the tool.
    + Channel Id for S32K148 is ``pcan`` and for POSIX is ``vcan0``. The ``--canif`` parameter is set to ``socketcan`` by default.
    + In the referenceApp the `ECU logical address` is ``0x2A`` and a possible `Client logical address` is ``0xEF0``.
    + You can find a reference canConfig.json file in ``tools/UdsTool/app/canConfig.json``.

UDS on Ethernet (DoIP)
++++++++++++++++++++++

To send UDS requests over Ethernet to the demo application running on POSIX, you can use the following commands:

- To send a Read Data By Identifier (RDBI) request for DID ``0xCF01``:

  ``udstool read --eth --host [Host IP] --ecu [ECU logical address] --source [Client logical address] --did cf01``

- To send a Write Data By Identifier (WDBI) request for DID ``0xCF03`` with data ``999``:

  ``udstool write --eth --host [Host IP] --ecu [ECU logical address] --source [Client logical address] --did cf03 --data 999``

- To send a Diagnostic Session Control request to switch to default session:

  ``udstool session --eth --host [Host IP] --ecu [ECU logical address] --source [Client logical address] --id 01``

.. note::
    + In referenceApp the `Host IP` is set to ``192.168.0.201`` for POSIX targets and ``192.168.0.200`` for S32K1xx targets.
    + In the referenceApp the `ECU logical address` is ``0x2A`` and a possible `Client logical address` is ``0xEF1``.
    + The tool supports DoIP protocol (ISO 13400) version 2(2012) and version 3(2019). The default version is set to 2.
      You can select the desired version using the ``--doip`` option followed by ``2`` or ``3``.
