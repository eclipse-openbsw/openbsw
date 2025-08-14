.. _LogicalAddress:

LogicalAddress helper functions
===============================

In many projects, we have the routine task of handling a message based on its source and/or target address:

* components which handle a bus have to check the logical address of an incoming request against a list of addresses, then perform an action based on the result of this check. For example, a doip server will only accept a request from a client whose logical address is part of a limited set of authorized addresses.
* routing has to direct messages from one bus to another, but some buses are addressed through wide (16-bit e.g. eth/doip) addresses, while others use cropped (8-bit e.g. CAN) addressing.

This generates the need for some basic functionality:

* encoding the two sides (wide/cropped) of a logical address
* creating a list of addresses
* determining if a wide or cropped address is part of an address list
* converting between wide and cropped addresses

In order to standardize these common operations, we created some helpers in `LogicalAddress.h/cpp`

Logical address
---------------

Tester addresses are encoded in a simple type:

.. code:: c++

    struct LogicalAddress
    {
        uint16_t addressDoip;
        uint16_t address8Bit;
    };

Creating list of addresses
--------------------------

Create the address lists as arrays of `LogicalAddress`, separate for each bus:

.. code:: c++

    //define address lists, usually in you project's TransportConfiguration.h
    static constexpr size_t const COUNT_OF_ETH_TESTERS          = 4;
    static constexpr size_t const COUNT_OF_CAN_TESTERS          = 2;
    static constexpr size_t const COUNT_OF_FLEXRAY_TESTERS      = 1;
    static constexpr size_t const COUNT_OF_ADDRESS_LISTS        = 3;
    static constexpr std::array<LogicalAddress, COUNT_OF_ETH_TESTERS> const
        TESTER_ADDRESS_RANGE_ETH
        = {{{0x0ECDU, 0x00CDU}, {0x0E11U, 0x0011U}, {0x0EE2U, 0x0017U}, {0x0ED5U, 0x0005U}}};
    static constexpr std::array<LogicalAddress, COUNT_OF_CAN_TESTERS> const
        TESTER_ADDRESS_RANGE_CAN
        = {{{0x1234U, 0x00CDU}, {0x0333U, 0x0033U}}};
    static constexpr std::array<LogicalAddress, COUNT_OF_FLEXRAY_TESTERS> const
        TESTER_ADDRESS_RANGE_FLEXRAY
        = {{0xFEF5U, 0x00F5U}};

Address search
--------------

Helper functions to check if an address is part of a specific list. For example in doip:

.. code:: c++

    bool allowRoutingActivation = addressfinder::isDoipAddressIn(address, TESTER_ADDRESS_RANGE_ETH)

Same operation on a cropped bus:

.. code:: c++

    bool testerKnown = addressfinder::is8BitAddressIn(address, TESTER_ADDRESS_RANGE_CAN)

Address conversions
-------------------

Helper class ``LogicalAddressConverter`` should be used to make conversions between wide and cropped addresses.
Since it is a template it is better to alias it:

.. code:: c++

    //type alias, usually in you project's TransportConfiguration.h
    using LogicalAddressConverterGateway = LogicalAddressConverter<COUNT_OF_ADDRESS_LISTS>;

Registering tester addresses with `LogicalAddressConverter`
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Address lists must be registered with ``LogicalAddressConverter``. This allows ``LogicalAddressConverter`` to search in every list for the correct conversion.

.. code:: c++

    //register lists with LogicalAddressConverter helper class, usually in your project's TransportConfiguration.cpp
    template<>
    std::
        array<::etl::span<LogicalAddress const>, TransportConfiguration::COUNT_OF_ADDRESS_LISTS> const
            TransportConfiguration::LogicalAddressConverterGateway::TESTER_ADDRESS_LISTS
        = {TransportConfiguration::TESTER_ADDRESS_RANGE_ETH,
        TransportConfiguration::TESTER_ADDRESS_RANGE_CAN,
        TransportConfiguration::TESTER_ADDRESS_RANGE_FLEXRAY};

Usage
+++++

When we receive a message in router, we can convert between wide/cropped addressing using ``convertDoipAddressTo8Bit`` and ``convert8BitAddressToDoip``, allowing messages to flow between buses addressed differently:

.. code:: c++

    if(targetBusId == ::busid::BusId::CAN())
    {
        uint16_t croppedTargetAddress = LogicalAddressConverterGateway::convertDoipAddressTo8Bit(targetAddress);
        pTransportMessage->setTargetId(croppedTargetAddress);
    }

If the address passed is not found in any list, the unchanged argument will be returned.