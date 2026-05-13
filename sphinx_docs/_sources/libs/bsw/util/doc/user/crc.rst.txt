.. _util_crc:

`util::crc`
===========

**CRC** (Cyclic Redundancy Check) codes are a class of error-detecting codes, which are a subset of
cyclic codes and, in turn, a subset of linear block codes. The primary purpose of CRC codes is to
detect accidental changes in data, such as during transmission. This is achieved by adding redundancy
in the form of a checksum to the transmitted data. If the checksum of the received data matches
the calculated checksum, it is assumed that no errors occurred during
transmission. However, this assumption is not always valid, as certain errors divisible by
the generator polynomial cannot be detected.

Theory behind CRC calculations
------------------------------
The redundancy is added by performing a polynomial division over the Galois Field *GF(2)*.
In order to be able to calculate this redundancy, a so called **generator polynomial** *g(x)* is
needed.
This polynomial is the *divisor* in this polynomial division, which takes the data as the
*dividend* and in which the *quotient* is discarded and the *remainder* becomes the result.
The generator polynomial should also be chosen to maximize the error-detection while minimizing the
collision probability.
The most important property of this generator polynomial is its length, which is determined by the
degree of the polynomial. This property also influences the length of the computed checksum,
i.e. the remainder of the polynomial division.
Examples how this polynomial division is performed can be seen here
`<https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks>`_.

Commonly used polynomial lengths are:

========= =========
Length    CRC width
========= =========
9 bits    CRC-8
17 bits   CRC-16
33 bits   CRC-32
========= =========

Therefore an *n* bit CRC can calculate an *n* bit checksum and the polynomial has degree *n*
with a length of *n* +1. It seems that the polynomial would not fit into a primitive data type
(`uint8_t`, etc.), but it can, as there are two coefficients which are always one, the highest ==
:math:`x^{n}`, and the lowest == :math:`x^{0}` == 1 positions.
Following this, there are four different ways how a generator polynomial can be written.
In the "normal" representation the highest coefficient is set as the implicit one. For example if
the generator polynomial is :math:`x^{3} + x + 1`, it could be represented as `0x103`, but as the
highest coefficient is always one, it is sufficient to write `0x03`. Please have a look at the
Wikipedia article for the other notations.

Selecting an appropriate CRC width
----------------------------------
The error detection effectiveness depends on several factors, including:

- the size of the data to be protected,
- messages per time frame,
- the types of errors to be expected,
- the smallest number of bit errors that are undetectable by the CRC code (Hamming Distance [HD]).

Therefore no one-size fits all approach can be used. Given the requirements, a CRC should be chosen
which fits these best. A starting point can be the `overview of the "best" CRC sizes
<https://users.ece.cmu.edu/~koopman/crc/index.html>`_ (assumption: low constant random independent
bit error rate (BER)) together with the Hamming Distance [HD].
For example, a CRC-8 is needed which detects 3-bit errors. HD=4 is chosen, as this means, that all
1, 2, ..., n-1 bit errors will be detected. According to the table the generator polynomial 0x83,
in normal notation 0x107 (without highest coefficient: 0x07), is the best polynomial for HD=4 and
for word length up to 119 bits. Above that size, there will be codewords which will be undetectable.

An example which highlights the above factors on error detection (from Dr. Koopman):
Assume 72000 messages/hour with each message consisting of 64 bytes.
The packet error ratio (PER) can be calculated with:
:math:`1 - (1 - 10^{-8})^{64*8} = 5,12*10^{-6}` errors/message
where :math:`10^{-8}` is the bit error probability.
Applying this value again in the PER gives:
:math:`1 - (1 - 5,12*10^{-6})^{72000} = 0,3083` errors/hour.
So if for example the requirement says that the packet error rate should be below :math:`10^{-9}`,
then a suitable CRC should be applied:

- CRC-16 with HD=1: :math:`1 - (1 - (5,12*10^{-6} * 2^{-16}))^{72000} = 5,625*10^{-6}` still not good enough.
- CRC-32 with HD=1: :math:`1 - (1 - (5,12*10^{-6} * 2^{-32}))^{72000} = 8,792*10^{-11}` this should suffice.

Select a CRC implementation
---------------------------
There are three possibilities how CRCs can be calculated:

1. Table based calculation: Fast, but bigger memory consumption (RAM/ROM)
2. Runtime calculation: Less memory consumption (RAM/ROM), but slower
3. Hardware supported calculation (device specific): Fastest method

This module implements the table based approach with pre-calculated tables. Therefore the RAM usage
is lower, but the ROM usage is increased.

Security considerations
-----------------------
The security goal of CRCs is to have an easy and fast way to check the **integrity** of messages.
CRCs provide **no** security methods for **confidentiality** or **authenticity**. An attacker could
change the message and the checksum without notice. For stronger security requirements other
ways should be considered, for example stream-ciphers with hash calculations (AEAD).

Overview CRC Module
-------------------

CRC configurations
++++++++++++++++++
The CRC template class has six different parameters which can be used to implement a CRC.
The parameters are:

1. CRC width (``uint8_t``, ``uint16_t``, ``uint32_t``)
2. the generator polynomial (`hex`)
3. the initial value of the remainder (`hex`)
4. if the input is reflected (``bool``)
5. if the output is reflected (``bool``)
6. value of the final XOR (`hex`)

.. note::
    The values for parameters two, three and six can also be given in decimal or octal, but
    hexadecimal is the preferred way.

Parameter one sets the width of the CRC calculation and parameter two specifies the generator
polynomial. The generator polynomial is set in the "normal" representation, i.e. if
:math:`x^{3} + x + 1` is the polynomial, then `0x3` is the representation in hex
(without the highest coefficient,
as it always equals to one).
Some applications have a different bit ordering, i.e. the bits are reversed from
Most-Significant-Bit (MSB) to Least-Significant-Bit (LSB) or vice versa. Therefore parameters four
and five can be used to match these requirements for input or output data. Additionally, some
standards set an initial value for the remainder and apply some final value which is XORed with the
CRC calculation. Parameters three and six can be used to apply these values.

How to calculate a CRC checksum
+++++++++++++++++++++++++++++++

.. warning::
    It is very important that the exact same CRC configuration is used for the CRC calculation and
    the CRC verification.

There are already some CRC configurations for different sizes in the header files. If the needed
CRC configuration is not available, see the next subsection on how to add a new configuration.
The usage is as follows: Define an instance of the needed CRC. Call the ``.init()`` method. Feed
the data into the ``.update()`` method and retrieve the checksum with ``.digest()``.

.. literalinclude:: ../../examples/Crc8Example.cpp
   :start-after: EXAMPLE_START crc8example
   :end-before: EXAMPLE_END crc8example
   :language: c++

How to add a new CRC configuration
++++++++++++++++++++++++++++++++++
At first the necessary configuration information (generator polynomial, input/output reflected, etc.
) of the CRC which should be implemented must be retrieved. A good starting point is the
CRC-Catalogue
(see below at :ref:`crc-further-resources`).

There are three possibilities how new CRC configurations can be added:

1. Permanently add a new CRC configuration, LUTs available
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If the generator polynomial and the corresponding Look-Up-Tables are available (look under
`src/util/crc/LookupTable_0x<polynomial>.cpp`), then just a new CRC configuration to the struct
can be added. See for example the `Rohc` entry in the `Crc8.h` header file.

.. code-block:: cpp

    struct Crc8
    {
        using Ccitt = CrcRegister<uint8_t, 0x07, 0>;
        using Rohc  = CrcRegister<uint8_t, 0x07, 0xff, true, true, 0>;
        // ...
        // Other CRC-8 configurations
        // ...
    }

2. Permanently add a new CRC configuration, no LUTs available
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If the generator polynomial and the LUTs are not there, a new entry in the specific header files,
i.e. for 8, 16, or 32 bit CRCs and the necessary Look-Up-Table (LUT) must be added.
Pre-calculate the entries for the LUT and add them in a new file under
`src/util/crc/LookupTable_0x<polynomial>.cpp`. Add also an entry to the corresponding header file.

.. code-block:: cpp

    struct Crc8
    {
        // ...
        // Other CRC-8 configurations
        // ...
        using Crc8F_3 = CrcRegister<uint8_t, 0xCF, 0>;
    }

3. Temporarily use a new CRC configuration, LUTs must be available
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Directly use the template class and create a new instance if it is just another configuration for
an already available polynomial. Just include the `util/crc/Crc.h` header file and use it as
follows:

.. literalinclude:: ../../examples/Crc8RohcExample.cpp
   :start-after: EXAMPLE_START crc8rohc
   :end-before: EXAMPLE_END crc8rohc
   :language: c++

FAQ
---
**Is a specific CRC check sequence of a specified width always the same for other CRC check
sequences of the same width?**

No it is not. There are many different configurations for e.g. 8-bit CRCs. It mainly depends on
the generator polynomial, but also on other criteria, i.e. if the input/output is
reflected, or a final XOR value is applied.

.. _crc-further-resources:

Further Resources
-----------------
 * `CRC Quick Information by Dr. Koopman <http://checksumcrc.blogspot.com/2020/06/crc-information-quick-start.html>`_
 * `A painless guide to crc error detection algorithms <http://www.zlib.net/crc_v3.txt>`_
 * `CRC on Wikipedia <http://en.wikipedia.org/wiki/Cyclic_redundancy_check>`_
 * `Online CRC-Calculator <http://www.sunshine2k.de/coding/javascript/crc/crc_js.html>`_
 * `Online CRC-Calculator for 8, 16, and 32 bit lengths <https://crccalc.com/>`_
 * `CRC-Catalogue <https://reveng.sourceforge.io/crc-catalogue/all.htm>`_
 * `Good CRC Tutorial <http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code>`_
