.. _util_format:

format - Printf-like String Formatting
======================================

This module provides classes to format text strings from data. The main intention is to simplify
writing textual data on output streams.

Example
-------

The ``::util::stream::IOutputStream`` object is not suitable for receiving formatted text
due to its input accepting only bytes and byte buffers. Whenever readable text should be written to
an output stream a ``::util::format::StringWriter`` object can be instantiated that wraps
the stream and provides printf-like functions for writing:

.. code-block:: cpp

    ::util::format::StringWriter writer(stream);
    stream.printf("This is a formatted int value: %d", int32Value).endl();

Features
--------

StringWriter
++++++++++++

The ``::util::format::StringWriter`` encapsulates most of the functionality of this sub
module.  Methods are provided for formatting data with a format string and data arguments in a
platform independent style. See ``::util::format::PrintfFormatter`` for a detailed
description of the format strings.

SharedStringWriter
++++++++++++++++++

Whenever ``::util::stream::ISharedOutputStream`` objects are provided for temporary mutual
exclusive use one cannot simply write onto the shared stream. The
``::util::format::SharedStringWriter`` provides a simple way to

+ start output (lock the stream exclusively) on creation
+ provide the ``::util::format::StringWriter`` functionality upon this temporary allocated
  stream
+ end output (release the stream) on destruction

Vt100AttributedStringFormatter
++++++++++++++++++++++++++++++

On small devices the most simple output stream typically is the stdout console stream. While
supported by terminal programs it may be useful to decorate text data by applying VT100 text
attributes to parts of it.  The ``::util::format::Vt100AttributedStringFormatter`` writes
VT100 format strings for switching on/off attributes and keeps track of all attributes being applied
to a stream. One can instantiate a VT100 formatter object and apply attributes to the text like:

.. code-block:: cpp

    ::util::format::StringWriter writer(stream);
    ::util::format::Vt100AttributedStringFormatter vt100;

    stream.printf("This is a formatted and attributed int value: ")
        .apply(vt100.attr(::util::format::RED))
        .printf("%d", int32Value)
        .apply(vt100.reset())
        .endl();

Reusable funtionality
+++++++++++++++++++++

A typical printf/sprintf implementation is a monolithic block that scans a format string and applies
it directly to the provided data. This makes the reuse of only parts of the functionality hard. The
Printf functionality is splitted into scanning format strings
(``:::util::format::PrintfFormatScanner``) and performing output on data arguments
(``::util::format::PrintfFormatter``) by providing arbitrary data
(``::util::format::IPrintfArgumentReader``).

