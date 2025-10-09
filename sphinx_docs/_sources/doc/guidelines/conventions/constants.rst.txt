Constants
=========

.. toctree::
   :maxdepth: 2
   :caption: Contents:

.. _coding_style_cpp_const_correctness:

Const Correctness
-----------------

``const`` should be used wherever data does not need to be mutable. It prevents
erroneous mutation of data and allows for potential compiler optimizations.

``const`` should be placed to the right of the type it modifies (east-const).
See also this `talk by Jon Kalb`_.

.. _talk by Jon Kalb: https://www.youtube.com/watch?v=fv--IKZFVO8

**By default...**

.. glossary::

    make objects immutable
        Immutable objects are easier to reason about, so make objects non-const only
        when there is a need to change their value. Prevents accidental or
        hard-to-notice change of value.

    make member functions const
        A member function should be marked const unless it changes the object’s
        logical state. This gives a more precise statement of design intent,
        better readability, more errors caught by the compiler, and sometimes
        more optimization opportunities. There's a good example of `logical vs. physical const on
        isocpp.org <https://isocpp.org/wiki/faq/const-correctness#logical-vs-physical-const>`_.

    use const to define objects with values that do not change after construction
        Prevent surprises from unexpectedly changed object values.

Function Declarations and Definitions
-------------------------------------

When a variable which is passed to a function by value is not modified in a function, it should be
marked in the parameter list of the function *definition* as ``const``. However, this variable
should **not** be marked ``const`` in the function *declaration*, as this ``const`` in the function
*declaration* is not used by the compiler and thus has no impact on the compiled code:

.. code-block:: cpp

    void func(size_t len); // good
    ...
    void func(const size_t len)
    {
        // ...
    }

    void func(const size_t len); // bad, const does nothing here
    ...
    void func(const size_t len)
    {
        // ...
    }
