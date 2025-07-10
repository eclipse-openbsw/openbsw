.. _util_string:

`util::string`
==============

This submodule contains helper classes that support string handling.

Overview
--------

Currently, this submodule contains only the ``util::string::ConstString`` class that represents
a pointer to char data, string size and a set of methods for comparison to other strings, getting
char data, size, finding characters, and similar operations.

String slices
-------------

When working with human-readable strings (such as formatting data), null-terminated character
arrays are typically used. However, if there is a need to work with slices of strings
(e.g., when parsing command-line statements), they are difficult to represent without copying these
parts into new buffers that include the null terminator.

The ``util::string::ConstString`` class provides a straightforward implementation for representing
non-C strings by holding a char pointer and a string length (number of characters). It can be
initialized with either null-terminated strings or a char pointer/length pair, as shown below:

.. code-block:: cpp

    ::util::string::ConstString str1("abcd");
    ::util::string::ConstString str2("abcdefg", 4);

In the example above both objects str1 and str2 represent the String "abcd".

.. note::
    Never assume that the strings are zero-terminated. Whenever you need the data of a
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

The ``util`` module provides the ``util::format::StringWriter`` class, which includes a
platform-independent printf implementation. This implementation also supports ConstString objects
using the formatter '%S'. However, since references to ``util::string::ConstString`` objects cannot
be placed directly into the variable argument list, you must use the
``util::string::ConstString::plain_str()`` method to obtain an appropriate data object:

.. code-block:: cpp

    void printString(IOutputStream & stream, ::util::string::ConstString str1)
    {
        ::util::format::StringWriter writer(stream);
        writer.printf("str = %S", str1);
        // alternatively:
        writer.write("str = ").write(str1);
    }
