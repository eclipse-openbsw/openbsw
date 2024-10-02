.. _UdsTool:

UdsTool
=======

Overview
--------

A simple tool to talk to ECUs via UDS (ISO 14229-1).

Installation
------------
Inside the ``tools/UdsTool`` folder run:

        ``pip install .``

Usage
-----
You can view all the options available by executing the following command:

    ``udstool --help``

The tool has the following structure:

    ``udstool [OPTIONS] COMMAND [ARGS]...``

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
