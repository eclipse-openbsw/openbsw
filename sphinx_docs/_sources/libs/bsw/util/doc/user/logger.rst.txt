.. _util_logger:

`util::logger`
==============

The `logger` submodule provides a straightforward interface for submodules to emit log messages.
It offers a set of methods for handling log messages while leaving the implementation details to
the application. The application determines how to filter and output received messages.
By default, without concrete implementation, log message calls are ignored.

Levels
------

A log message first has a given severity (level) that indicates the importance of the message.
There's a fixed set of levels defined (with descending importance):

+----------------+-----------------------------------------------------------------------------+
| Level          | Description                                                                 |
+================+=============================================================================+
| critical       | Any error that is hard to recover from or that indicates severe data loss   |
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

As a general rule, the info log level should be the default without generating an excessive
amount of logs. It is good practice to avoid repetitive logging of the same state and instead
focus on logging state changes.

Logger Components
-----------------

With many modules covering diverse functionality, it is crucial to have an identifier that indicates
the origin of a log message. Therefore, each message must be assigned to a logger component to
specify its source. Typically, a logging implementation allows filtering messages by components.

Defining a logger component
+++++++++++++++++++++++++++

When a module emits logs, the key decision is determining which `logger component`
to assign to the message. Typically, a module defines a uniquely named `logger component`.
This definition must reside in a source file because the `logger component` is mapped to a global
``uint8_t`` variable. For instance, the ``TEST_COMPONENT`` `logger component` can be defined in a
``TestComponentLogger.cpp`` file as follows:

.. code-block:: cpp

    #include "util/logger/Logger.h"

    DEFINE_LOGGER_COMPONENT(TEST_COMPONENT)

.. important::
    The logger component declaration and definition macros create components within the
    ``util::logger`` namespace. It is crucial to use these macros only outside of any namespace
    and not from within one!

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

If no dedicated header file contains the logger component's declaration, the
``DECLARE_LOGGER_COMPONENT`` macro can be placed in any other source or header file where the
component is used, provided the declaration is made outside of any namespace.

If a log is used from within a source file it might make sense to place a ``using`` declaration
to the top of the file that allows unqualified usage of the component for logging:

.. code-block:: cpp

    using ::util::logger::TEST_COMPONENT;

Functionality
-------------

The method-based API is mainly based on the class ``util::logger::Logger`` that is a
simple facade that can propagate calls to a logging implementation.

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

    package "Logger implementation 1" {
        ComponentMapping1 <|-- IComponentMapping
        LoggerOutput1 <|-- ILoggerOutput
    }

    package "Logger implementation 2" {
        ComponentMapping2 <|-- IComponentMapping
        LoggerOutput2 <|-- ILoggerOutput
    }

    Logger ..> ComponentMapping1
    Logger ..> LoggerOutput1

    Logger ..> ComponentMapping2
    Logger ..> LoggerOutput2

Usage
+++++

When emitting logs from within a source file, it is advisable to include a using directive at
the top of the file. This allows unqualified usage of both the ``util::logger::Logger`` class
and the `logger component`:

.. code-block:: cpp

    using ::util::logger::Logger;
    using ::util::logger::TEST_COMPONENT;


A log message is emitted by calling a log method with the component and a ``printf``-style text,
as shown:

.. code-block:: cpp

    Logger::info(TEST_COMPONENT, "Using version %d.%02d", major, minor);


In the above example, the severity is specified using the static method
``util::logger::Logger::info``, and the `logger component` is ``util::logger::TEST_COMPONENT``.
The additional arguments have effect similar to arguments in the ``printf`` function.

In rare cases where the severity is not fixed, the more generic method
``util::logger::Logger::log`` can be used:

.. code-block:: cpp

    Logger::log(TEST_COMPONENT, getLevel(), "Using version %d.%02d", major, minor);


If preparing arguments for emitting a log is time-consuming, it may be advisable to
first check whether logging is enabled for the given component and desired log level.
A log can then be emitted as follows:

.. code-block:: cpp

    if (Logger::isEnabled(::util::logger::LEVEL_INFO))
    {
        Logger::info(TEST_COMPONENT, "Time consuming log of value %d", getComputedValue());
    }

Initialization and shutdown
+++++++++++++++++++++++++++

The method-based logger API is just a facade that can propagate log messages to a logger
implementation -- if provided. For collecting log messages two interfaces have to be provided:

+ A so called component mapping interface (implementation of
  ``util::logger::IComponentMapping``) that filters logs and holds readable names for the
  components and levels
+ A logger output interface (implementation of ``util::logger::ILoggerOutput``) that
  receives already filtered log  messages and information about the component and the level

Having decided and instantiated all necessary interfaces, the logger then has to be initialized with
a call to:

.. code-block:: cpp

    Logger::init(componentMapping, loggerOutput);

In case there's a regular shutdown scenario the logger implementation can be disconnected by a call
to the shutdown method like:

.. code-block:: cpp

    Logger::shutdown();

Synchronous/Asynchronous logging
++++++++++++++++++++++++++++++++

The logger output is, by design, a synchronous call (see ``util::logger::Logger::doLog()`` method).
However, the subsequent ``util::logger::ILoggerOutput::logOutput()`` interface method can be
implemented according to the needs of the application.
For example, the ``logger::BufferedLoggerOutput`` class (:ref:`logger`) initially stores
messages in a buffer. These messages are then popped and sent to the output stream by calling the
``logger::BufferedLoggerOutput::outputEntry()`` method, which can, in turn,
be invoked in a loop within the OS's lower-priority tasks of the application.

Timestamps
++++++++++

The logger interface does not provide timing output.
Typically, appropriate timestamp information is added in the implementation of the
``util::logger::ILoggerOutput::logOutput()`` method of the ``util::logger::ILoggerOutput`` interface.