Bus identification traits
=========================

Overview
--------

The module provides ``::common::busid::BusIdTrait`` class for bus identification. The only trait supported now is the name of the bus.

Examples
--------

.. code-block:: cpp

    int busID = busid::CAN_0; // busID is a CAN bus
    LOGSYNCHRON("\r\n busID %d : %s  -> ", busID, ::common::busid::BusIdTraits::getName(busID));