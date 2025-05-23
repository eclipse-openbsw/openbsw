.. _util_stream:

`util::stream`
==============

This module provides classes to format text strings from data. The class simplifies
printing textual data into output streams.

util::stream::Std(in/out)Stream
-------------------------------

Classes ``util::stream::StdinStream`` and ``util::stream::StdoutStream`` are tightly connected to
the byte-by-byte input and output functions ``getByteFromStdin()`` and ``putByteToStdout()`` that
in turn are architecture-dependent, i.e. their implementation is found in corresponding BSP.

.. code-block:: cpp

    StdoutStream cut;
    cut.write_string_view(::etl::string_view("Test"));

util::stream::NormalizeLfOutputStream
-------------------------------------

The ``util::stream::NormalizeLfOutputStream`` class is implemented using the `decorator` design
pattern. It inherits from the ``util::stream::IOutputStream`` interface and aggregates an instance
of ``util::stream::StdoutStream``. The ``write()`` function of this class also checks newline symbols
(Windows, Linux) to ensure proper output formatting.

.. code-block:: cpp

    declare::StringBufferOutputStream<40> stream;
    NormalizeLfOutputStream cut(stream, "[CRLF]");
    cut.write('a');
    cut.write('\n');
    // The symbol of new line is replaced by CR/LF:
    assert("a[CRLF]" == std::string(stream.getString()));

util::stream::NullOutputStream
------------------------------

The class is also inheriting from ``util::stream::NormalizeLfOutputStream`` and providing dummy
``write()`` methods implementations.

.. code-block:: cpp

    NullOutputStream stream;
    assert(stream.isEof());

util::stream::SharedOutputStream
--------------------------------

The class implements three specific methods: ``startOutput()``, ``endOutput()``, and
``releaseContinuousUser()``.

- The ``startOutput()`` method checks if another stream user is currently utilizing the output stream. If so, it forces the user to finish its output before returning the shared stream.
- The ``endOutput()`` method resets the internal user pointer to ``nullptr`` (default) or to a specified user.
- The ``releaseContinuousUser()`` method combines the functionality of the first two methods: it forces the current stream user to finish and then resets the internal user pointer to ``nullptr``.

.. code-block:: cpp

    void outputShared(ISharedOutputStream::IContinuousUser& user)
    {
        declare::StringBufferOutputStream<80> stream;
        SharedOutputStream sos(stream);
        assert(&stream == &cut.sos(user));
        stream.releaseContinuousUser(user);
    }

util::stream::StringBufferOutputStream
--------------------------------------

The ``util::stream::StringBufferOutputStream`` class is useful for outputting data into a string
buffer instead of printing it to the terminal. This class inherits from
``util::stream::ISharedOutputStream`` and provides the same interface methods (``write()``,
``isEof()``, etc.) as other output stream classes.

.. code-block:: cpp

    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).subslice(9));
    cut.write_string_view(::etl::string_view("abc"));
    cut.write_string_view(::etl::string_view("def"));
    assert("abcdef" == cut.getString());

util::stream::TaggedSharedOutputStream
--------------------------------------

The ``util::stream::TaggedSharedOutputStream`` class follows the decorator design pattern,
inheriting from (among others) the ``util::stream::ISharedOutputStream`` class and encapsulating an
instance thereof. It enhances the methods of the ``util::stream::ISharedOutputStream`` class by
adding specific starting (prefix) and ending (suffix) textual information when outputting to the stream.

.. code-block:: cpp

    ::util::stream::declare::StringBufferOutputStream<80> bufferStream;
    ::util::stream::TaggedSharedOutputStream cut(bufferStream, "[START]", "[CRLF]");
    bufferStream.write('a');
    bufferStream.write('\n');
    bufferStream.write_string_view(::etl::string_view("abc\ndef"));
    bufferStream.write_string_view(::etl::string_view("AB\nDEF"));
    cut.endOutput(nullptr);
    assert("[START]a[CRLF][START]abc[CRLF]" == bufferStream.getString());
