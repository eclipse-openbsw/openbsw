.. _util_logger:

logger - logger API
===================

The logger API submodule contains a simple API that allows submodules to emit log messages. It does
not provide any implementation (a logger framework) for handling of the log messages. It is up to an
application to decide about how to filter and output received messages. By default (without any
framework) log message calls are simply ignored.

Levels
------

A log message first has a given severity (level) that indicates the importance of the message.
There's a fixed set of levels defined (with descending importance):

+----------------+-----------------------------------------------------------------------------+
| Level          | Description                                                                 |
+================+=============================================================================+
| fatal/critical | Any error that is hard to recover from or that indicates severe data loss   |
+----------------+-----------------------------------------------------------------------------+
| error          | Any error of a service that can cause data loss and which cannot be handled |
|                | automatically                                                               |
+----------------+-----------------------------------------------------------------------------+
| warn           | Events or unexpected behaviour that can be handled automatically            |
+----------------+-----------------------------------------------------------------------------+
| info           | General useful information to log, such as normally handled user events or  |
|                | state changes.                                                              |
+----------------+-----------------------------------------------------------------------------+
| debug          | Information that is diagnostically helpful                                  |
+----------------+-----------------------------------------------------------------------------+

It is up to the developer to decide about assignment of severities to a message. But as a thumb rule
info should be selectable as default log level without obtaining tons of logs. It is always good
practice to avoid repetitive logging of the same state but focussing on logging of state changes.

Logger Components
-----------------

While there are a lot of modules which cover very different functionality it is crucial to have an
identifier that defines about the origin of a log message. Therefore to each message a logger
component has to be assigned to indicate that origin. Typically a logging framework allows filtering
messages by components.

Defining a logger component
+++++++++++++++++++++++++++

When logs should be emitted by some module the most important decision is about the logger component
assigned to the message. Typically a module will define a uniquely named logger component. This
definition has to be placed into a source file because the logger component definition will be
mapped to a global ``uint8_t`` variable. The definition of a ``TEST_COMPONENT`` logger component
could be placed in a ``TestComponentLogger.cpp`` file like:

.. code-block:: cpp

    #include "util/logger/Logger.h"

    DEFINE_LOGGER_COMPONENT(TEST_COMPONENT)

.. note::
    The logger component declaration and definition macros create the components in the namespace
    ``::util::logger``. It is crucial to not use the define or declare macros from within
    a namespace but only outside of any namespace!

To allow users to access the logger component (typically when configuring a logger in an application
main file) it is good style to also add a header file that contains the declaration of the logger as
follows:

.. code-block:: cpp

    #ifndef TEST_COMPONENT_LOGGER_H
    #define TEST_COMPONENT_LOGGER_H

    #include "util/logger/Logger.h"

    DECLARE_LOGGER_COMPONENT(TEST_COMPONENT)

    #endif // TEST_COMPONENT_LOGGER_H

Using a logger component
++++++++++++++++++++++++

If there's no include file that holds a logger component's declaration, the declaration using
``DECLARE_LOGGER_COMPONENT`` can also be placed in any other source or header file where the component
is to be used, as long as the declaration is done outside of any namespace.

If a log is used from within a source file it might make sense to place a ``using`` statement to the
top of the file that allows unqualified usage of the component for logging:

.. code-block:: cpp

    using ::util::logger::TEST_COMPONENT;


Macro-based API
---------------

There's a macro based API that allows specifying logs using macros which can arbitrarily be defined
by a logging framework.

Value based log macros
++++++++++++++++++++++

A value based log consists of two or more values. The first value is the component identifier and the
second is the message (a zero-terminated string value). All additional data can simply be added by
appending values which can be named and also equipped with a unit.

This might look complicated but has the advantage that message and data are strictly separated and
can be interpreted by value.

================================== =======================
Macro                              Description
================================== =======================
``LOG_F(component, message, ...)`` Log with severity fatal
``LOG_E(component, message, ...)`` Log with severity error
``LOG_W(component, message, ...)`` Log with severity warn
``LOG_I(component, message, ...)`` Log with severity info
``LOG_D(component, message, ...)`` Log with severity debug
================================== =======================

Formatted message log macros
++++++++++++++++++++++++++++

A formatted message consists of a message (a zero-terminated string value) that contains
``printf``-like format specifiers for embedding additional values.

===================================== =========================================
Macro                                 Description
===================================== =========================================
``LOG_FMT_F(component, format, ...)`` Log formatted message with severity fatal
``LOG_FMT_E(component, format, ...)`` Log formatted message with severity error
``LOG_FMT_W(component, format, ...)`` Log formatted message with severity warn
``LOG_FMT_I(component, format, ...)`` Log formatted message with severity info
``LOG_FMT_D(component, format, ...)`` Log formatted message with severity debug
===================================== =========================================

Atomic value macros
+++++++++++++++++++

Atomic value macros add type information to atomic values.

==================== ======================================== ======================
Macro                Type                                     Comment
==================== ======================================== ======================
``L_BOOL(val)``      ``bool``
``L_I8(val)``        ``int8_t``
``L_U8(val)``        ``uint8_t``
``L_H8(val)``        ``uint8_t``                              output as hex
``L_I16(val)``       ``int16_t``
``L_U16(val)``       ``uint16_t``
``L_H16(val)``       ``uint16_t``                             output as hex
``L_I32(val)``       ``int32_t``
``L_U32(val)``       ``uint32_t``
``L_H32(val)``       ``uint32_t``                             output as hex
``L_I64(val)``       ``int64_t``
``L_U64(val)``       ``uint64_t``
``L_H64(val)``       ``uint64_t``                             output as hex
``L_STR(val)``       ``const char*``                          zero-terminated string
``L_SIZED_STR(val)`` ``::util::string::ConstString``          sized string
``L_PTR(val)``       ``const void*``                          pointer
==================== ======================================== ======================

Array value macros
++++++++++++++++++

Array value macros add type information to array values.

===================== =========================================== =============
Macro                 Type                                        Comment
===================== =========================================== =============
``L_ARRAY_BOOL(val)`` ``::estd::slice<const bool>``
``L_ARRAY_I8(val)``   ``::estd::slice<const int8_t>``
``L_ARRAY_U8(val)``   ``::estd::slice<uint8_t const>``
``L_ARRAY_H8(val)``   ``::estd::slice<uint8_t const>``            output as hex
``L_ARRAY_I16(val)``  ``::estd::slice<const int16_t>``
``L_ARRAY_U16(val)``  ``::estd::slice<const uint16_t>``
``L_ARRAY_H16(val)``  ``::estd::slice<const uint16_t>``           output as hex
``L_ARRAY_I32(val)``  ``::estd::slice<const int32_t>``
``L_ARRAY_U32(val)``  ``::estd::slice<const uint32_t>``
``L_ARRAY_H32(val)``  ``::estd::slice<const uint32_t>``           output as hex
``L_ARRAY_I64(val)``  ``::estd::slice<const int64_t>``
``L_ARRAY_U64(val)``  ``::estd::slice<const uint64_t>``
``L_ARRAY_H64(val)``  ``::estd::slice<const uint64_t>``           output as hex
===================== =========================================== =============

Other macros
++++++++++++

+-------------------------+----------------------------------------------------------------------+
| Macro                   | Description                                                          |
+=========================+======================================================================+
| ``L_NAME(name, value)`` | names the value (zero-terminated string). ``value`` is another value |
|                         | macro                                                                |
+-------------------------+----------------------------------------------------------------------+
| ``L_UNIT(unit, value)`` | provide a unit (zero-terminated string). ``value`` is another value  |
|                         | macro                                                                |
+-------------------------+----------------------------------------------------------------------+
| ``L_FMT(format, ...)``  | string value specified as a printf-like call where ``format`` is the |
|                         | string that can contain format specifiers for additional parameters  |
+-------------------------+----------------------------------------------------------------------+

Examples
++++++++

Value based error with named string value and named integer value:

.. code-block:: cpp

    LOG_E(FLEXRAY, "A voltage error has occurred",
                L_NAME("reason", L_STR("unknown")),
                L_NAME("voltage, L_UNIT("mV", L_U32(voltage))));

Value based info with named hex parameters:

.. code-block:: cpp

    LOG_I(TRANSPORT, "Transport message received",
                L_NAME("src", L_H16(src)),
                L_NAME("dst", L_H16(dst)));

Debug log with additional formatted parameter:

.. code-block:: cpp

    LOG_D(TEST_COMPONENT, "IPv4 address received",
                L_FMT("%d.%d.%d.%d", L_U8(addr[0]), L_U8(addr[1]), L_U8(addr[2]), L_U8(addr[3])));

Printf-like message with formatted string and integer:

.. code-block:: cpp

    LOG_FMT_E(FLEXRAY, "A voltage error has occurred with reason=%s and voltage=%d mV",
                L_STR("unknown"),L_U32(voltage));

Printf-like message with formatted hex parameters:

.. code-block:: cpp

    LOG_FMT_I(TRANSPORT, "Transport message received with src=%02x and dst=%02x",
                L_U16(src), L_U16(dst));


Attaching a logging framework
+++++++++++++++++++++++++++++

To attach a logging framework (which can define the macros) it is sufficient to provide a file
``util/logger/LoggerBinding.h`` on an include path that precedes the include path of module
``util``.  Here all API macros can be defined (overwritten). Macros which aren't defined here will
be defined void by the logging API.

.. note::
    The macro-based API is recommended for new modules because it allows better optimizations (e.g.
    for suppressing certain log messages). There is currently only the ``dlt`` module that supports a
    complete implementation for value based logging.


Method-based API
----------------

The method-based API is mainly based on the class ``::util::logger::Logger`` that is a
simple facade that can propagate calls to a logging framework.

Class diagram
+++++++++++++

.. uml::
    :scale: 100%

    Logger : log()
    Logger : critical()
    Logger : error()
    Logger : warn()
    Logger : info()
    Logger : debug()
    Logger : isEnabled()
    Logger : getLevel()

    Module1 ..> Logger
    Module2 ..> Logger

    class IComponentMapping
    class ILoggerOutput

    package "Logger framework 1" {
        ComponentMapping1 <|-- IComponentMapping
        LoggerOutput1 <|-- ILoggerOutput
    }

    package "Logger framework 2" {
        ComponentMapping2 <|-- IComponentMapping
        LoggerOutput2 <|-- ILoggerOutput
    }

    Logger ..> ComponentMapping1
    Logger ..> LoggerOutput1

    Logger ..> ComponentMapping2
    Logger ..> LoggerOutput2

Usage
+++++

When logs are emitted from within a source file it is advisable to place a ``using`` statement to
the top of the file that allows unqualified usage of both class ``::util::logger::Logger``
and logger component:

.. code-block:: cpp

    using ::util::logger::Logger;
    using ::util::logger::TEST_COMPONENT;


Emitting a log message is then simply calling a log method with the component and a printf-style
text like:

.. code-block:: cpp

    Logger::info(TEST_COMPONENT, "Using version %d.%02d", major, minor);


In the above example the severity is expressed by usage of the static method
``::util::logger::Logger::info`` and the logger component is called
``::util::logger::TEST_COMPONENT``. The additional arguments is like a simple ``printf``.

In rare cases the severity will not be fixed and for this the more generic method
``::util::logger::Logger::log`` can be used:

.. code-block:: cpp

    Logger::log(TEST_COMPONENT, getLevel(), "Using version %d.%02d", major, minor);


In case the preparation of arguments for emitting a log would be time consuming it can make sense to
first check whether logging is enabled for the given component and the desired log level. So a log
would be emitted like follows:

.. code-block:: cpp

    if (Logger::isEnabled(::util::logger::LEVEL_INFO))
    {
        Logger::info(TEST_COMPONENT, "Time consuming log of value %d", getComputedValue());
    }

Providing a logger framework
++++++++++++++++++++++++++++

The method-based logger API is just a facade that can propagate log messages to a logger
implementation -- if provided. For collecting log messages two interfaces have to be provided:

+ A so called component mapping interface (implementation of
  ``::util::logger::IComponentMapping``) that filters logs and holds readable names for the
  components and levels
+ A logger output interface (implementation of ``::util::logger::ILoggerOutput``) that
  receives already filtered log  messages and information about the component and the level

Having decided and instantiated all necessary interfaces the logger than has to be initialized with
a call to:

.. code-block:: cpp

    Logger::init(componentMapping, loggerOutput);

In case there's a regular shutdown scenario the logger implementation can be disconnected by a call
to the shutdown method like:

.. code-block:: cpp

    Logger::shutdown();

Synchronous/Asynchronous logging
++++++++++++++++++++++++++++++++

The logger output by design is synchronously i.e. the logOutput method of the interface
``::util::logger::ILoggerOutput`` will be called immediately if the filtering was
successful. Thus it is up to the implementation to decide whether to emit the message in a
synchronous or asynchronous way.

Timestamps
++++++++++

Provision and handling of timestamps is not addressed in the logger interface. Typically appropriate
timestamp information will be added in the implementation of the ``logOutput`` method of interface
``::util::logger::ILoggerOutput``.
