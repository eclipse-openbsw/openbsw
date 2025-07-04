Enumerations
============

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Concept
-------

An enumeration describes a set of `named values`. These `named values` or `enumerators` are usually
identifiers that behave as constants.
A variable which has been declared as having an enumerated type can be assigned any of the
enumerators as a value.
These enumerators can be compared and assigned, but usually the type of a concrete representation
of these enumerators was not declared in the past.
Since C++11 it is possible to define the underlying type of the enumeration.
There is a distinction between `unscoped` and `scoped` enumerations also since C++11.
Only scoped enumerations should be used for projects using C++11 or upwards.
More information can be found `here <https://en.cppreference.com/w/cpp/language/enum>`_.

Unscoped Enumerations
---------------------
Unscoped enumerations should **not** be used anymore.
The rationale behind this is that the underlying type is implementation-defined by the compiler.
The only restriction is that the type must be able to represent the enumeration values.
Usually this type will be implemented as an ``int`` or ``unsigned int``.

.. code-block:: cpp

    // violations: A7-2-2, A7-2-3
    enum Colors
    {
        RED,
        GREEN,
        BLUE,
        ...
    };

    // violation: A7-2-3
    enum Colors : uint8_t
    {
        RED,
        GREEN,
        BLUE,
        ...
    };

Scoped Enumerations
-------------------
Since C++11 scoped enumerations can and should be used.
In general, scoped enumerations are better in the sense that the C++ standard requires from the
compiler that scoped enumerations must be implemented with the underlying type ``int``.
Even though this requirement for the compilers is an advantage, the size of type ``int`` is still
platform dependent. Therefore the type should be directly defined.
In order to save memory, the smallest possible representation **should** be used, which can be in
most cases ``uint8_t``.

.. code-block:: cpp

    // violation: A7-2-2
    enum class Colors
    {
        RED,
        GREEN,
        BLUE,
        ...
    };

    // recommended definition
    enum class Colors : uint8_t
    {
        RED,
        GREEN,
        BLUE,
        ...
    };

One advantage is that variables can now be compared with the scope resolution operator.
A drawback is that there is no direct conversion from the values of a scoped enumerator to the
underlying type.
Use ``::etl::to_underlying`` from ``etl/utility.h`` to cast the enumeration value to the
underlying type.

.. code-block:: cpp

    #include <etl/utility.h>

    // recommended definition
    enum class Colors : uint8_t
    {
        RED,
        GREEN,
        BLUE,
    };

    Colors color = Colors::BLUE;

    switch(color)
    {
        case Colors::RED    : printf("Red color\n"); break;
        case Colors::GREEN  : printf("Green color\n"); break;
        case Colors::BLUE   : printf("Blue color\n"); break;
    }

    // option 1: (recommended) use the etl helper function
    auto const colorRed = ::etl::to_underlying(Colors::RED);
    // option 2: explicitly cast using the underlying type
    auto const colorGreen = static_cast<::std::underlying_type<Colors>::type>(Colors::GREEN);
    // option 3: explicitly cast to the specified type
    auto const colorBlue = static_cast<uint8_t>(Colors::BLUE);
    // uint8_t colorBlue = color; // error: no implicit conversion from scoped enum to uint8_t

    printf("Red as uint8_t: %d\n", colorRed); // will print "0"
