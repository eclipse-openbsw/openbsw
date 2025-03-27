.. _estd_little_endian:

little_endian
=============

Overview
--------

The `little_endian` is a utility designed to work with binary data and provides a convenient and
type-safe way to convert between data types and their little-endian representations. It is a
collection of functions and structures designed to facilitate the handling of data in byte
representation. The least significant byte is stored at the lowest memory address.

Usage
-----

.. list-table::
    :widths: 30 70

    * - Constraints
      - The functions are designed for different integer sizes (e.g., 8-bit, 16-bit, 32-bit, and
        64-bit). |br| |br| Recommended to use with correct underlying data type. Otherwise data
        could be wrongly interpreted.
    * - Usage guidance
      - The little-endian data is stored in an array of bytes. |br| |br| The size of this
        array is determined by the size of the data type being represented. |br| |br| For example,
        if user is working with a 32-bit integer, the 'little_endian' will have an array of 4
        bytes to store the data in little-endian format.

Examples
--------

The following examples show how the little_endian can be used for various operations :

read_le
+++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN read le
    :end-before: EXAMPLE_END read le
    :language: c++
    :dedent: 4

write_le
++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN write le
    :end-before: EXAMPLE_END write le
    :language: c++
    :dedent: 4

operator=
+++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN assignment
    :end-before: EXAMPLE_END assignment
    :language: c++
    :dedent: 4

operator T()
++++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN cast to primitive
    :end-before: EXAMPLE_END cast to primitive
    :language: c++
    :dedent: 4

make
++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN make
    :end-before: EXAMPLE_END make
    :language: c++
    :dedent: 4

make_le
+++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN make le
    :end-before: EXAMPLE_END make le
    :language: c++
    :dedent: 4

read_le_24
++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN read le 24
    :end-before: EXAMPLE_END read le 24
    :language: c++
    :dedent: 4

write_le_24
+++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN write le 24
    :end-before: EXAMPLE_END write le 24
    :language: c++
    :dedent: 4

read_le_48
++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN read le 48
    :end-before: EXAMPLE_END read le 48
    :language: c++
    :dedent: 4

write_le_48
+++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN write le 48
    :end-before: EXAMPLE_END write le 48
    :language: c++
    :dedent: 4

read_le_bits
++++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN read le bits
    :end-before: EXAMPLE_END read le bits
    :language: c++
    :dedent: 4

write_le_bits
+++++++++++++

.. sourceinclude:: examples/little_endian.cpp
    :start-after: EXAMPLE_BEGIN write le bits
    :end-before: EXAMPLE_END write le bits
    :language: c++
    :dedent: 4
