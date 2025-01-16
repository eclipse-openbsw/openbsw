.. _util_meta:

`util::meta`
============

util::meta::BinaryValue
-----------------------

``BinaryValue`` template struct allows creating integers using another uit32_t integer,
representing binary value as argument for template. This class is similar to binary
representation as string in ``std::bitset``.

.. code-block:: cpp

    uint32_t binaryValue   = BinaryValue<11>::value;
    assert(3 == binaryValue);

util::meta::Bitmask
-------------------

The ``util::meta::Bitmask`` template struct facilitates the convenient generation of an integer
value whose binary representation consists entirely of ones (1) in all positions.

.. code-block:: cpp

    assert(uint16_t(0x000F) == (Bitmask<uint16_t, 4>::value)); // 1111
