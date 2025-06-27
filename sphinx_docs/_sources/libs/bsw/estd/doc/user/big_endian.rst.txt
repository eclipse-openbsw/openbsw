.. _estd_big_endian:

big_endian
==========

Overview
--------

Endianness is the order of bytes of a word of digital data in computer memory.
The two main types of endianness is **Big Endian (BE)** and **Little Endian
(LE)**. In **Big Endian**, the most significant bit (MSB) is stored first (i.e.
in the lowest memory address).

For example, on a big-endian computer, Consider an integer 0x12345678 stored in
a 32-bit big-endian system. The bytes would be arranged in memory as:

+---------+-------+
| Address | Value |
+=========+=======+
| 100     | 0x12  |
+---------+-------+
| 101     | 0x34  |
+---------+-------+
| 102     | 0x56  |
+---------+-------+
| 103     | 0x78  |
+---------+-------+


Usage
-----

The "big_endian" stores the data in an array of bytes. The size of the array is
determined by the size of the data type being represented. For example, a
16-bit integer, the "big_endian" will have an array of 2 bytes to store the data
in "big_endian" format.

Example
-------

The example shows reading data in big-endian format from buffer array.

.. literalinclude:: ../../examples/big_endian.cpp
   :start-after: EXAMPLE_READ_U16_START
   :end-before: EXAMPLE_READ_U16_END
   :language: c++
   :dedent: 4

.. literalinclude:: ../../examples/big_endian.cpp
   :start-after: EXAMPLE_WRITE_U32_START
   :end-before: EXAMPLE_WRITE_U32_END
   :language: c++
   :dedent: 4