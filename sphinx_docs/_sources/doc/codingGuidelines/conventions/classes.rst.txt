Classes
=======

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Constructor
-----------

Constructors must have empty bodies. Class members and parent classes must be initialized in
the initialization list of a constructor.

**Rationale:** While it might seem tempting to put initialization logic in constructors,
this significantly increases the effort of checking if the logic failed. In embedded systems
exceptions are typically either disabled or not allowed, thus to handle an error which
occurred during object construction it is necessary to save information about the failure
and check for such information after the object has been constructed.

.. code-block:: cpp

    // good
    class TcpConnection
    {
    public:
        enum class ReturnCode : uint8_t
        {
            TCP_OK,
            TCP_HANDSHAKE_FAILED,
            TCP_CONNECTION_LOST,
            ...
        };

        TcpConnection(ip const& addr, uint16_t port);
        ReturnCode init();

    private:
        ip       _addr;
        uint16_t _port;
    };

    TcpConnection::TcpConnection(ip addr, uint16_t port)
    : _addr(addr)
    , _port(port)
    {
        // do nothing
    }

    TcpConnection::ReturnCode TcpConnection::init()
    {
        if (!establishConnection(addr, port))
        {
            return TcpConnection::ReturnCode::TCP_CONNECTION_LOST;
        }

        if (!performHandshake(addr, port))
        {
            return TcpConnection::ReturnCode::TCP_HANDSHAKE_FAILED;
        }

        ...

        return TcpConnection::ReturnCode::TCP_OK;
    }

    int main()
    {
        TcpConnection tcpConnection(ip(127.0.0.1), 30501);

        // good - action and getting its result are done in the same line
        auto rc = tcpConnection.init();
        if (rc != TcpConnection::ReturnCode::TCP_OK)
        {
            // handle the error
        }

        ...
    }

as opposed to

.. code-block:: cpp

    // bad
    class TcpConnection
    {
    public:
        enum class ReturnCode : uint8_t
        {
            TCP_OK,
            TCP_HANDSHAKE_FAILED,
            TCP_CONNECTION_LOST,
            ...
        };

        TcpConnection(ip const& addr, uint16_t port);

        // bad: additional getter function to retrieve a return code
        ReturnCode getLastReturnCode() const;

    private:
        ip         _addr;
        uint16_t   _port;
        ReturnCode _lastRc; // bad: additional data member to store a return code
    };

    TcpConnection::TcpConnection(ip addr, uint16_t port)
    : _addr(addr)
    , _port(port)
    {
        if (!establishConnection(_addr, _port))
        {
            _lastRc = TcpConnection::ReturnCode::TCP_CONNECTION_LOST;
            return;
        }

        if (!performHandshake(_addr, _port))
        {
            _lastRc = TcpConnection::ReturnCode::TCP_HANDSHAKE_FAILED;
            return;
        }

        ...

        _lastRc = TcpConnection::ReturnCode::TCP_OK;
    }

    TcpConnection::ReturnCode TcpConnection::getLastReturnCode() const
    {
        return _lastRc;
    }

    int main()
    {
        // bad - the action is performed during the construction of an object
        TcpConnection tcpConnection(ip(127.0.0.1), 30501);

        ... // do whatever we want while program execution is potentially in a state of failure

        // ... and retrieving the result happens on a separate line
        if (tcpConnection.getLastReturnCode() != TcpConnection::ReturnCode::TCP_OK)
        {
            // handle the error
        }

        ...
    }

**Exception:** Having logic in constructors is allowed in the RAII idiom as it's implied by design.

Destructor
----------

| *A base class destructor should be either public and virtual, or protected and non-virtual.*
| (Herb Sutter in the  C/C++ Users Journal, September 2001)

In embedded systems with static memory allocation, there is never a use case for deleting objects
through a pointer to a base class. Since a virtual destructor adds to the vtable of a class, it
might add a considerable amount of ROM usage to an applications binary, especially if a class
is subclassed very often (e.g. ``::async::IRunnable``).

:rule:`CPP-024` The destructor of an C++ interface should be protected non virtual.
    This makes sure that code, which would delete an object through a pointer to an interface
    will not compile.

:rule:`CPP-025` A class, which is not intended to be subclassed should be declared final and have
public non-virtual destructor.

    This makes it clear to users of the code that a class is not designed to be a base class.
    This is the default in our embedded applications. In other words: if no subclass exists in the
    code base, a class should most likely be declared final.

Example (Interface):

.. code-block:: cpp

    class IDemo
    {
    protected:
        ~IDemo() = default;
    public:
        virtual void foo() = 0;
        ...
    };

Example (Non-base class):

.. code-block:: cpp

    template<class T, size_t N>
    class vector final : public vector<T>
    {
    public:
        ~vector() = default;

        size_type size();
        ...
    };

For more information and examples see `CERT rule OOP52 <https://wiki.sei.cmu.edu/confluence/display
/cplusplus/OOP52-CPP.+Do+not+delete+a+polymorphic+object+without+a+virtual+destructor>`_.


Getters, Setters, Attribute Functions
-------------------------------------

In general ``set`` functions should be avoided, when possible. Instead, classes
should be initialized when calling their constructor. To read properties of a
class, also avoid ``get`` functions. Instead use ``const`` attribute style
functions.

.. code-block:: cpp

    class CanTransceiver    // good
    {
    public:
        CanTransceiver(BusId id);

        BusId busId() const;
    };

    class CanTransceiver    //  bad
    {
    public:
        void setBusId(BusId id);

        BusId getBusId() const;
    };


Boolean Member Functions
------------------------

Boolean member functions tend to be `const`.

  .. code-block:: cpp

      class CanTransceiver
      {
      public:
          bool hasWakeupOccurred() const;
      };

Class Invariant
---------------

Use ``class`` if the class has an invariant, use ``struct`` if the data members
can vary independently.

Readability, ease of comprehension, the use of class alerts the programmer to
the need for an invariant.

.. note::

    An invariant is a logical condition for the members of an object that a
    constructor must establish for the public member functions to assume.
    After the invariant is established (typically by a constructor) every
    member function can be called for the object. An invariant can be stated
    informally (e.g., in a comment) or more formally using Expects.

    If all data members can vary independently of each other, no invariant is
    possible.

The following example shows a definition with independent member variables,
hence ``struct`` is used

.. code-block:: cpp

    struct Pair  // members can vary independently
    {
        string name;
        int volume;
    };

but in contrast

.. code-block:: cpp

    class Date
    {
    public:
        // validate that {yy, mm, dd} is a valid date
        Date(int yy, Month mm, char dd);

    private:
        int _year;
        Month _month;
        char _day;
    };

contains an invariant, thus it should be a ``class`` definition.

This rule quotes directly `rule C.2 of the C++ Core Guidelines
<https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#user-content-c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently>`_.

Uncopyable
----------

To prevent objects of a certain type from being copied, explicitly mark its copy constructor and
assignment operator as deleted:

.. code-block:: cpp

    class Example
    {
    public:
        Example() = default;

        Example(Example const&) = delete;
        Example& operator=(Example const&) = delete;
    };

Making a type uncopyable should be a conscious decision and have a good justification - it is not
the default choice. You should also consider if the type can be movable or not, and if needed
also disable move construction:

.. code-block:: cpp

    Example(Example &&) = delete;
    Example& operator=(Example&&) = delete;


Before the adoption of the C++11 standard, ``UNCOPYABLE`` macro from ``estd`` library was used.


Indestructible
--------------

To avoid destruction of objects, use the ``etl::typed_storage`` wrapper. The type will
become `trivially destructible
<http://www.cplusplus.com/reference/type_traits/is_trivially_destructible/>`_.

In the following example the object of type `YourType` will be constructed with placement-new,
but never destructed even when `wrapper` gets destructed. The destructor of `YourType` will never be
called, except if you call destroy() explicitly:

.. code-block:: cpp

    etl::typed_storage<YourType> wrapper;
    wrapper.create();


Avoid Protected Data
--------------------

Protected data is a source of complexity and errors. Protected data complicates the statement of
invariants. Protected data inherently violates the guidance against putting data in base classes,
which usually leads to having to deal with virtual inheritance as well.

However, protected member functions can be just fine.


.. _class_hierarchies_are_bad:

Prefer Concrete Types Over Class Hierarchies
--------------------------------------------

A concrete type is fundamentally simpler than a hierarchy: easier to design, easier to implement,
easier to use, easier to reason about, smaller, and faster. You need a reason (use cases) for using
a hierarchy.

If a class can be part of a hierarchy, it encourages manipulation of its objects through pointers
or references. That implies more memory and run-time overhead to perform the resulting indirections.

**Do not use inheritance when simply having a data member will do.** Usually this means that the
derived type would need to override a base virtual function or needs access to a protected member.

This rule quotes directly `rule C.10 of the C++ Core Guidelines
<https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#user-content-c10-prefer-concrete-types-over-class-hierarchies>`_.


Use =default and =delete Wherever Possible
------------------------------------------

Since C++11 constructors, destructors, and certain operators can be marked ``=default`` or
``=delete`` - which expresses choosing the default, compiler-generated implementation, or blocking
the default behavior.

Use ``=default`` if you want to be explicit about using the default semantics. This method is
preferred over defining e.g. empty constructors/destructors or trivial assignment operators.
The compiler is more likely to get the default semantics right and you cannot implement these
functions better than the compiler.

Use ``=delete`` when you want to disable default behavior (without wanting an alternative), like
e.g. blocking copy construction. Note that deleted functions should be public.

This rule quotes `rules C.80 and C.81 of the C++ Core Guidelines
<https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#user-content-c80-use-default-if-you-have-to-be-explicit-about-using-the-default-semantics>`_.

Virtual Functions Should Specify Exactly one of Virtual, Override, or Final
---------------------------------------------------------------------------

Virtual functions should be used with caution and only when necessary - as they introduce a layer
of indirection in the function call. They have drawbacks both for humans (making it harder to reason
about the code execution flow) and for machines (enforcing indirect function call, taking space for
``vtable``, prohibiting the compiler from optimizing your code, etc.).

Whenever declaring a virtual function, **one** of the following keywords should be used:

- ``virtual`` means exactly and only "this is a new virtual function."
- ``override`` means exactly and only "this is a non-final overrider."
- ``final`` means exactly and only "this is a final overrider."

Functions marked ``virtual``, should be pure virtual (marked with ``= 0``). Providing default
implementations and creating complex class hierarchies goes against
:ref:`the previous rule<class_hierarchies_are_bad>`.


Ordering
--------

A conventional order of members improves readability. When declaring a class
use the following order:

  1. types: classes, enums, and aliases (using, typedef)
  2. constructors, assignments, destructor
  3. functions
  4. data

Use the ``public`` before ``protected`` before ``private`` order.

.. code-block:: cpp
    :caption: example (shortened)

    class FrameProvider
    {
    public:
        enum class ErrorCode : uint8_t
        {
            PROVIDER_ERR_FRAME_SENT,
            PROVIDER_ERR_FRAME_NOT_SENT
        };

        FrameProvider();

        void init(FrameProviderContainer& container);
        void shutdown();
        void cyclicTask();
        // ...

    private:
        friend class FrameProviderContainer; // friend classes should not be used

        enum class ProviderState : uint8_t
        {
            EMPTY,
            INITIALIZED,
            WAITING_FOR_FLOW_CONTROL,
            READY_FOR_CONSEC_FRAMES,
            WAITING_FOR_TX_CALLBACK,
            NO_MORE_FRAMES_AVAILABLE
        };

        void unlock(ProcessingResult status);
        void asyncUnlock(ProcessingResult status);

        // ...
        CANFrame _txFrame;
        ProviderState _state;
        // ...
    };
