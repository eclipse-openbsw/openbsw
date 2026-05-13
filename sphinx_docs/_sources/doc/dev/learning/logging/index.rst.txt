.. _learning_logging:

Logging to the console
======================

Previous: :ref:`learning_lifecycle`

The simplest way to learn about the logging system in BSW based projects is to look at a working example.

Search the code for occurrences of these macros...

* ``DECLARE_LOGGER_COMPONENT``
* ``DEFINE_LOGGER_COMPONENT``
* ``LOGGER_COMPONENT_MAPPING_INFO``

You will find that ``DECLARE_LOGGER_COMPONENT`` is used in header files to declare a logging component
(providing an ``extern`` reference to it)
and these header files are included in any code with log messages that use that logging component.
You will find that ``DEFINE_LOGGER_COMPONENT`` is defined once per logging component in a ``.cpp`` file
to instantiate the logging component.

eg. ``libs/bsw/cpp2can/include/can/CanLogger.h`` contains...

.. code-block:: cpp

    #include "util/logger/Logger.h"

    DECLARE_LOGGER_COMPONENT(CAN)

and ``libs/bsw/cpp2can/src/can/CanLogger.cpp`` contains...

.. code-block:: cpp

    #include "can/CanLogger.h"

    DEFINE_LOGGER_COMPONENT(CAN)

With these in place, log messages can use the ``CAN`` logging component at different output levels.
For example, ``platforms/posix/main/src/systems/CanSystem.cpp`` contains...

.. code-block:: cpp

    using ::util::logger::CAN;
    using ::util::logger::Logger;

    void CanSystem::canFrameSent(::can::CANFrame const& frame)
    {
        Logger::info(
            CAN,
            "[CanSystem] CAN frame sent, id=0x%X, length=%d",
            (int)frame.getId(),
            (int)frame.getPayloadLength());
    }

The macro ``LOGGER_COMPONENT_MAPPING_INFO`` is used to set up a table containing info such as the default levels for each logging component.
In this project, ``executables/referenceApp/application/src/logger/logger.cpp`` contains...

.. code-block:: cpp

    START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(loggerComponentInfoTable)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, BSP, ::util::format::Color::DEFAULT_COLOR)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, COMMON, ::util::format::Color::DEFAULT_COLOR)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, DEMO, ::util::format::Color::DEFAULT_COLOR)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, GLOBAL, ::util::format::Color::DEFAULT_COLOR)
    /* start: adding logger components */
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, LIFECYCLE, ::util::format::Color::DARK_GRAY)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, CONSOLE, ::util::format::Color::DEFAULT_COLOR)
    #ifdef PLATFORM_SUPPORT_CAN
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, CAN, ::util::format::Color::LIGHT_BLUE)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, DOCAN, ::util::format::Color::LIGHT_GRAY)
    #endif // PLATFORM_SUPPORT_CAN
    #ifdef PLATFORM_SUPPORT_UDS
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, UDS, ::util::format::Color::LIGHT_YELLOW)
    LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, TPROUTER, ::util::format::Color::LIGHT_YELLOW)
    #endif // PLATFORM_SUPPORT_UDS
    /* end: adding logger components */
    END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

To learn more,
you can try changing the default logging levels for components to see how this effects logging output,
or you could add your own logging component.
See :ref:`util_logger` for more information.

Next: :ref:`learning_commands`
