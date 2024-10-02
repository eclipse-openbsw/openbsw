.. _util_string:

string - String Handling
========================

This submodule contains helper classes that support string handling.

Overview
--------

Currently this submodule only contains the ``::util::string::ConstString`` class that
represents pairs of character array and size fields. It is the counterpart to the
``::util::buffer::ConstByteBuffer`` class when working with characters.


String slices
-------------

When working with human readable strings (such as formatting data) typically zero terminated char
arrays are used. If somewhere there's the need to work with slices of strings (such as needed when
e.g. parsing command line statements) they can hardly be represented without copying these parts
into new buffers that can also hold the terminating zero.

The ``::util::string::ConstString`` provides a simple implementation for representing non-C
strings by holding a char pointer and a string length (number of characters). It can be initalized
with both zero-terminated strings or a char pointer/length pair like:

.. code-block:: cpp

    ::util::string::ConstString str1("abcd");
    ::util::string::ConstString str2("abcdefg", 4);

In the example above both objects str1 and str2 represent the String "abcd".

.. note::
    Never assume that represented strings are zero-terminated. Whenever you need the data of a
    ConstString with a terminating zero you will need to copy it into an appropriate buffer and add
    the terminating zero on your own!


Comparison
----------

The ConstString class provides comparison methods (both case sensitive and case insensitive) as a
replacement of using ``::strcmp``/``::stricmp``:

.. code-block:: cpp

    ::util::string::ConstString str1("abcd");
    ::util::string::ConstString str2("abcdefg", 4);
    ::util::string::ConstString str3("AbCde", 4);

    estd_assert(str1.compare(str2) == 0);
    estd_assert(str1.compare(str3) != 0);
    estd_assert(str1.compareIgnoreCase(str3) == 0);

There are also comparison operators for binary comparisons (based on the case senstitive comparison
method):

.. code-block:: cpp

    ::util::string::ConstString str1("abcd");
    ::util::string::ConstString str2("abcdefg", 4);
    ::util::string::ConstString str3("abe");

    estd_assert(str1 == str2);
    estd_assert(str1 != str3);
    estd_assert(str1 < str3);


Using ConstStrings in printf arguments
--------------------------------------

The util module provides the ``::util::format::StringWriter`` class with a
platform-independent printf implementation. This implementation accepts also ConstString objects (by
providing the formatter '\%S').  While references to the ConstString objects themselves may not be
placed into the variable argument list, one has to use the plain_str() method to receive an
appropriate data object:

.. code-block:: cpp

    // ...

    ::util::format::StringWriter writer(stream);
    ::util::string::ConstString str1("abcdefg", 4);
    writer.printf("str = %S", str1.plain_str());
