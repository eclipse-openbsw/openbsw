.. _util_format:

`util::format`
==============

This module provides classes for formatting text strings from data. These classes simplify
printing textual data to output streams.

Example
-------

The ``util::stream::IOutputStream`` object is not suitable for printing formatted text,
as it processes characters as unformatted (raw) bytes. To write readable text to
the output stream, a ``util::format::StringWriter`` object can be instantiated.
This object wraps the stream and provides printing functions:

.. code-block:: cpp

  void ptintInteger(::util::stream::IOutputStream& outputStream, int int32Value)
  {
      ::util::format::StringWriter writer(outputStream);
      writer.printf("This is a formatted int value: %d", int32Value).endl();
  }

Features
--------

StringWriter
++++++++++++

The ``util::format::StringWriter`` encapsulates most of the functionality of this
submodule. It provides methods for formatting data using a format string and data arguments in a
platform-independent manner. For a detailed description of format strings, refer
to ``util::format::PrintfFormatter``.

SharedStringWriter
++++++++++++++++++

``util::format::SharedStringWriter`` is a class that implements the `RAII` idiom.
A local object of this type is created immediately before outputting data to the terminal,
typically within the same visibility scope, such as a function or block.

An object of this type shares a pointer to the stream, which may also be used by other terminal
clients. Upon creation, it checks if the stream is in use, forces it to finish, and captures the
stream for its own use. The stream is released when the object is destroyed.

`util::format::Vt100AttributedStringFormatter`
++++++++++++++++++++++++++++++++++++++++++++++

On small devices, the simplest output stream is typically the `stdout` console stream. While
supported by terminal programs, it can be useful to decorate text data by applying `VT100` text
attributes to certain parts. The ``util::format::Vt100AttributedStringFormatter`` class writes
`VT100` format strings to toggle attributes on or off and tracks all attributes applied
to a stream. A `VT100 formatter` object can be instantiated and used to apply attributes to text,
as shown:

.. code-block:: cpp

  void printText(::util::stream::IOutputStream& stream)
  {
      ::util::format::StringWriter writer(stream);
      ::util::format::Vt100AttributedStringFormatter vt100;

      writer.printf("This is a formatted and attributed int value: ")
          .apply(vt100.attr(::util::format::RED))
          .printf("%d", int32Value)
          .apply(vt100.reset())
          .endl();
  }

Variadic parameters
-------------------

The ``printf()`` function of ``util::format::StringWriter`` is a typical variadic function, meaning
it accepts any number of arguments (variadic parameters).
The format string is expected to be the first argument in such a call.
Subsequent arguments must match the placeholders (% symbols) in the format string.

The types of the arguments are restricted to standard (primitive) C types: integral types, pointers,
and characters. Integral types may be prefixed with characters specifying their numeral system
(decimal, octal, or hexadecimal).

The following classes are responsible for correctly handling variadic arguments:

* ``util::format::PrintfFormatScanner``
* ``util::format::PrintfFormatter``
* ``util::format::PrintfArgumentReader``

These classes handle parsing the variadic arguments, converting integral types to
their corresponding character representations, and printing the resulting bytes
into the stream. They can be used independently or through the
``util::format::StringWriter`` class (refer to the ``printf()`` example above).

.. code-block:: cpp

  void printText(::util::stream::IOutputStream& stream)
  {
      PrintfFormatter formatter(stream);
      formatter.format("%d %s", 17, "abc"); // expected "17 abc"
      formatter.format("%+012d", 47); // expected "+00000000047"
      formatter.format("%hx", 0x23c5); // expected "23c5"
  }
