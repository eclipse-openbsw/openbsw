.. _style_naming_conventions:

Naming Conventions
==================

.. toctree::
   :maxdepth: 2
   :caption: Contents:


General Naming Rules
--------------------

:rule:`CPP-001` All names should be in English
    The single exception to this rule is if there is a necessity to use names
    originating from non-English documentation.

:rule:`CPP-002` Use intention-revealing names
    The name of a variable, type or function should intuitively imply **why it
    exists and what it does**. Spending a considerable amount of time finding
    good names saves future time for everyone (including yourself) wanting to
    read and understand the code.

:rule:`CPP-003` As long as necessary - as short as possible
    When choosing names, aim for improving readability of the code. Names should
    be descriptive, but at the same time as short as possible.

    Adding meaningful context can help to better/faster understand your code.
    For example a variable named ``address`` is probably considered rather
    ambiguous. Adding more context to its name, e.g.  ``ipAddress`` or
    ``postalAddress`` makes it much easier to reason about this variable without
    even knowing which software module it is in.  On the other hand, overly
    complicated names and gratuitous context (e.g.
    ``BspEthernetControllerInstanceManager``) are hard to read and remember, and
    only increase the mental overhead in understanding the code. Also, keep in
    mind that type of the variable, the usage of well-known idioms (like naming
    a loop-counter variable ``i``), etc. also contribute to the context and help
    the reader.


Type Names
----------

:rule:`CPP-004` Names representing types must be written in `UpperCamelCase`_.

.. code-block:: cpp

    using TcpipEventManager = ::os::EventManager<task_tcpip>;

Any exceptions to this rule need to have a reasonable justification and in that
case should be applied consistently within a module (e.g. ``etl`` types use
*snake_case* notation in order to match with their STL counterpart). In general
those exceptions should be rare!

.. _variable_names:

Variable Names
--------------

:rule:`CPP-005` Variable names must be written in `lowerCamelCase`_ and should not carry any
prefix (except :ref:`class_member_variables`)

.. code-block:: cpp

    Particle higgsBoson;

.. warning::

    We *do not* use `Hungarian Notation`_.


Constant Names
--------------

:rule:`CPP-006` Constants with static storage duration, e.g. global ``const``, ``constexpr`` or
local ``static const``, are named using ``ALL_CAPITAL_LETTERS`` interleaved with underscores.

:rule:`CPP-007` All other constants, e.g. local non-static, follow the usual :ref:`variable_names`
convention.

.. code-block:: cpp

    const int SOME_GLOBAL_CONSTANT = 1337;

    class Life
    {
    public:
        static const MEANING = 42;

        bool halfTheTruth(const int value)
        {
            const int half = value / 2;
            return half;
        }
    };


Function Names
--------------

:rule:`CPP-008` Choose expressive names for (member-)functions and format them using
`lowerCamelCase`_.

Usually function names contain verbs reflecting what they are actually doing.
Thereby, choose one word per concept, e.g. do not mix ``fetch``, ``retrieve``
and ``get`` within the same context.

The type name of the object is implicit, and should be avoided in a method name.
Try to be precise without repeating yourself.  For example if you provide a
function to send a ``CanFrame`` just call it ``send`` instead of
``sendCanFrame``. The function signature already tells the user that this
function sends a ``CanFrame``.

.. code-block:: cpp

    // good
    class CanTransceiver
    {
    public:
        ErrorCode send(const CanFrame& frame);
        static bool addGlobalListener(ICanFrameListener& listener);
    };

    // bad
    class CanTransceiver
    {
    public:
        ErrorCode SendFrame(const CanFrame& frame);
        static bool AddGlobalListener(ICanFrameListener& listener);
    };


Classes
-------

.. _class_names:

Class Names
+++++++++++

:rule:`CPP-009` **Class names** are written in `UpperCamelCase`_.

    Also abbreviations like ``CAN`` are camel cased to ``Can`` .

:rule:`CPP-010` **Interface names** start prefixed with a capital ``I``.

:rule:`CPP-011` Names of **abstract classes** are not treated specially any
more, we used to prefix them with ``Abstract``.

:rule:`CPP-012` In unit tests, when writing a **mock**, the class name is usually
postfixed with `Mock`.

.. code-block:: cpp

    // good
    class CanTransceiver;             // regular class
    class CanTransceiverMock;         // according mock class
    class ITransportMessageProvider;  // interface

    // bad
    class CANTransceiver;             // abbreviation not camel case
    class filteredLinFrameListener;   // wrong capitalization
    class AbstractDiagJob;            // Abstract prefix is deprecated

.. seealso:: Global naming conventions in :ref:`style_naming_conventions`

.. _class_member_variables:

Member Variable Names
+++++++++++++++++++++

:rule:`CPP-013` Member variables of a ``class``, ``struct`` or ``union`` shall be prefixed with
an ``_`` (underscore) in order to distinguish them easily from local variables.

:rule:`CPP-014` If a ``struct`` or ``union`` is a `POD`_, i.e. all members are public and it
does not provide any functions, you can omit the prefix.

.. note::
    In **legacy code** you will find the prefix ``f`` a lot but also ``m`` or
    ``_``. You can keep these prefixes as long as you stay consistent within
    your legacy module. :rule:`CPP-015` **Never mix styles!**

.. code-block:: cpp

    class CanTransceiver    // good
    {
        ::etl::intrusive_forward_list<ICanFrameListener, ::etl::forward_link<0>> _listeners;
    };

    struct Result           // good
    {
        ::bsp::ErrorCode status;
        uint16_t value;
    };


    class CanTransceiver    // bad, no member prefix
    {
        ::etl::intrusive_forward_list<ICanFrameListener, ::etl::forward_link<0>> listeners;
    };

    struct Result           // bad, suffix and mixed style
    {
        ::bsp::ErrorCode Status;
        uint16_t value_;
    };


Namespace Names
---------------

:rule:`CPP-016` Names representing namespaces should be all lowercase.

.. code-block:: cpp

    model::analyzer
    io::iomanager
    common::math::geometry


File Names
----------

:rule:`CPP-017` Files containing classes should only contain a single class. The header and
source file are named according to that class in *PascalCase*, e.g. :file:`MyClass.h` and
:file:`MyClass.cpp`.

:rule:`CPP-018` The **extension** for C++ source files is :file:`.cpp`, the extension for C
source files is :file:`.c`, the extension for assembly files is :file:`.s`.
Header files have the extension :file:`.h` no matter which content.

:rule:`CPP-022` If a file contains no class but only free function declarations it should
have a meaningful name in *camelCase*, e.g. :file:`routingAddons.h`.

Module Names
------------

:rule:`CPP-023` Module names must be written in `lowerCamelCase`_.

.. code-block:: cpp

    // good
    doip
    cpp2Crypto
    osUtils
    noInitRam

    // bad
    cpp2can
    state_machine
    mymodule
    YourModule

Special Cases
-------------

Abbreviations in Names
++++++++++++++++++++++

:rule:`CPP-019` Abbreviations in code should be written to match the appropriate contextual
casing style to maintain style consistency and code readability.

.. code-block:: cpp

    class CANReader                  // bad, not using UpperCamelCase
    {
        uint32_t const can_bus = 10; // bad, not using UPPER_SNAKE_CASE
        void initCAN();              // bad, not using lowerCamelCase
    };

    class CanReader                  // good
    {
        uint32_t const CAN_BUS = 10; // good
        void initCan();              // good
    };


Boolean Variables and Functions
+++++++++++++++++++++++++++++++

- :rule:`CPP-020` Boolean variable and function names shall start with ``is`` or ``has``.
  Likewise, a function prefixed with ``has`` or ``is`` shall return a boolean.

  .. code-block:: cpp

      bool isTerminated = connection.terminate();   // good
      bool terminated = connection.terminate();     // bad

      bool hasValidPayload() const { ... }     // good
      bool checkPayload() { ... }              // bad

- :rule:`CPP-021` Negated boolean variable names must be avoided.

  The problem arises when such a name is used in conjunction with the logical
  negation operator as this results in a double negative. It is not immediately
  apparent what ``!isNotFound`` means.

- Try to find short and precise names for the variables or functions. It often depends on the
  context, e.g. a variable within a `Connection` class can be named ``isTerminated``, but in a
  broader context it might be good to name it ``isConnectionTerminated``.

  Generally, avoid misunderstandings about the meaning. Does it indicate that an error happened?
  Or that everything is good to go? If it's still unclear, document the variable or return value.

- An alternative is using an `enum`.

  Pros:

  - more expressive
  - can be extended

  Cons:

  - needs more typing
  - extending an enum can be dangerous if not handled properly by the callers of the function
  - might need conversion if propagated to higher levels

  Other rarely used types are `bitfields` and `ranges`. For return values of functions it is
  worth to look at ``::etl::expected`` class which combines data and error status in one object.

  In most cases using a boolean is the best choice to keep it simple.

Plural Forms Should Be Used on Names Representing a Collection of Objects
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Enhances readability since the name gives the user an immediate clue of the
type of the variable and the operations that can be performed on its elements.

.. code-block:: cpp

    using ReceiverList = ::etl::intrusive_forward_list<Receiver, ::etl::forward_link<0>>;

    vector<Point>  points;
    int            values[];

Be careful not to lie with names, i.e. do not name a collection of ``Point``
objects ``pointList`` unless it's actually a ``List``-type.

.. _Hungarian Notation: https://en.wikipedia.org/wiki/Hungarian_notation
.. _UpperCamelCase: https://en.wikipedia.org/wiki/Camel_case
.. _lowerCamelCase: https://en.wikipedia.org/wiki/Camel_case
.. _POD: https://en.cppreference.com/w/cpp/named_req/PODType
