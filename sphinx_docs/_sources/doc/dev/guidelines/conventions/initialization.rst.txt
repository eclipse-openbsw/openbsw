Initialization
==============

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Overview
--------

There are different ways to initialize variables, objects, and data members. Mixing or misusing them
could lead to confusing results or just look confusing to the reader.

**Types of initialization cheatsheet**:

.. code-block:: cpp

    int x(42);                      // direct initialization
    int y{42};                      // direct initialization (available in C++11)
    ip addr(127, 0, 0, 1);          // direct initialization (calling a constructor)
    int z = 42;                     // copy initialization
    int a = {42};                   // copy initialization (available in C++11)
    xyz pointA{x, y, z};            // direct-list-initialization
    xyz pointB = {x, y, b};         // copy-list-initialization
    vector<int> validIndices(3, 0); // direct initialization of vector with three zeros

    struct xyz {
        int x{0}; // non-static data member initialization (NSDMI)

        ...
    };

Direct Initialization
---------------------

Direct initialization is the way to initialize an object from an explicit set of constructor arguments.
Variables of built-in types should be initialized only with direct initialization (unless the variable
in question is a member of a class). This way of initialization should be used by default because
it's the least ambiguous.

Example:

.. code-block:: cpp

    class IpAddress
    {
    public:
        IpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

        ...
    };

    int main()
    {
        // direct initialization by calling a parameterized constructor
        IpAddress addr(127, 0, 0, 1);

        // direct initialization for built-in types
        uint16_t multicast(30490U);
        uint16_t port{30501};

        ...
    };


**Important:** Please use direct initialization with curly braces for built-in types as *it does
narrowing conversion checks* on most compilers.

List Initialization
-------------------

It is good to use list initialization *as long as it leaves no data uninitialized*.
**Partial initialization is not allowed**, since it can lead to ambiguity and confusion.

Example:

.. code-block:: cpp

    struct Coordinate
    {
        int32_t x;
        int32_t y;
    };

    struct ForecastPoint
    {
        Coordinate c;
        int8_t temperature;
    };

    int main()
    {
        // bad
        ForecastPoint p1{123, -15}; // warning: read as {{123, -15}, 0} by a compiler

        // good
        ForecastPoint p2{{123, 345}, -15};

        ...
    }

Value Initialization
--------------------

Avoid using value initialization. Use direct or direct-list-initialization instead.

*NOTE*: Value initialization has been valid since C++03. An attempt to use value initialization syntax
in older versions of C++ resulting in **undefined behavior**.

**Rationale:** It's confusing and hard to maintain value initialization without introducing bugs
into the code.

Example:

.. code-block:: cpp

    struct Box {
        int32_t x;
        int32_t y;
        uint32_t w;
        uint32_t h;
    };

    int main()
    {
        // bad

        // in C++03 and later causes all members to be initialized to zero
        auto zeroSizedBox = Box(); // uses default constructor and initializes data with zeros
        cout << zeroSizeBox.x; // OK in C++03, but undefined behavior in C++98

        // good
        Box anotherZeroSizedBox{0, 0, 0, 0};
    }

If someone adds a user-defined constructor to this class, then such constructor has to initialize
all the data members, otherwise it's **undefined behavior**.

Example:

.. code-block:: cpp

    struct Box {
        Box() { /* do nothing */ } // user-defined constructor

        int32_t x;
        int32_t y;
        uint32_t w;
        uint32_t h;
    };

    int main()
    {
        // bad: user-defined constructor doesn't initialize data members
        auto zeroSizedBox = Box();
        cout << zeroSizedBox.x; // undefined behavior
    }

If a constructor is present in a class declaration, but marked as *default*, then the example above
will not cause undefined behavior.

.. code-block:: cpp

    struct Box {
        Box() = default;

        int32_t x;
        int32_t y;
        uint32_t w;
        uint32_t h;
    };

    int main()
    {
        auto zeroSizedBox = Box(); // initializes everything with zeros
    }

**Warning:** However, defining a constructor as default *after* the declaration makes it seen as
user-defined by the compiler, hence changing the example above to cause **undefined behavior**.

.. code-block:: cpp

    struct Box {
        Box();

        ...
    };

    Box::Box() = default; // seen as user-defined by a compiler

    int main()
    {
        auto zeroSizedBox = Box();
        // WARNING: access to any member field leads to undefined behavior
    }

Non-static Data Member Initialization (NSDMI)
---------------------------------------------

At all times all data members of a class should be initialized in a constructor initializer list if
such a constructor is user-defined and *not* defaulted. If the constructor of a class is not provided or
declared as default then NSDMI may take place.
*NOTE*: It's important to know that in this case such a class is no longer
trivially constructible.

.. code-block:: cpp

    // good (NOTE: struct isn't trivial anymore)
    struct Point2d
    {
        int32_t x{0};
        int32_t y{0};
    };

    // good (same as above)
    struct Point3d
    {
        Point3d() = default;

        int32_t x{0};
        int32_t y{0};
        int32_t z{0};
    };

    // good (NOTE: struct is trivial)
    struct Vector2d
    {
        int32_t x;
        int32_t y;
    }

    // good
    struct Vector3d
    {
        Vector3d(float inX, float inY, float inZ)
        : x(inX)
        , y(inY)
        , z(inZ)
        {}

        float x;
        float y;
        float z;
    };

    // bad - mixed initialization
    struct Location
    {
        Location(): latitude(1000), longitude(2000) { ... }
        int32_t latitude{0};
        int32_t longitude{0};
    };

**Rationale:** When initialization is done both in NSDMI and the constructor's initializer list it's hard to read,
especially in cases where class members aren't declared consecutively but spread across the
class definition. On the contrary, when initialization is done in one place (constructor) it's easy to see what
is initialized with what.

Conclusion
----------

Using direct initialization is the least ambiguous in comparison with other ways to initialize
variables, objects, and data members.
If a variable is declared as *auto*, trying to initialize it with copy-initialization may be ambiguous and
confusing:

.. code-block:: cpp

    auto temperature{36.6};      // type of temperature is double, but
    auto temperatureF = {97.88}; // type of temperatureF is std::initializer_list<double>

*Copy-list-initialization* can differ from *direct-list-initialization*
depending on the C++ standard of the compiler and becomes consistent only starting from *C++20*, thus
*copy-list-initialization* should be avoided.

.. code-block:: cpp

    int answer = {42}; // bad

The way of initialization above *should not be used* both for behavior and appearance consistency.

* *Direct initialization* / *direct-list-initialization* is the preferred way to initialize built-in type variables.

.. code-block:: cpp

    int answer{42}; // good

* *List initialization* is the preferred way to initialize trivial objects or containers, if possible.

.. code-block:: cpp

    Box a = Box();            // bad: value initialization
    Box b = { 1, 2, 10, 15 }; // bad: initializer_list
    Box c = {};               // bad: causes undefined behavior under certain conditions

    Box d{1};                 // bad: partial initialization / implicit initialization with zeros
                              //      of everything except for the first data member

    Box d{1, 2, 10, 15};      // good: all data members are initialized
