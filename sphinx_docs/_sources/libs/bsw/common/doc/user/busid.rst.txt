Bus Identification Traits
=========================

Overview
--------

The module provides ``::common::busid::BusIdTraits`` class for bus identification.
The only trait supported now is the name of the bus.

Example
-------

.. code-block:: cpp

    auto busID = busid::CAN_0; // busID is a CAN bus
    printf("busID %d : %s  -> ...\n", busID, ::common::busid::BusIdTraits::getName(busID));
