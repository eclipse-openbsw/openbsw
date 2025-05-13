Generic printing utilities
==========================

Overview
--------

The module provides provides ``LOGSYNCHRON`` macros - generic printing into terminal.

Examples
--------

.. code-block:: cpp

    int busID = busid::CAN_0; // busID is a CAN bus
    LOGSYNCHRON("\r\n busID %d : %s  -> ", busID, ::common::busid::BusIdTraits::getName(busID));