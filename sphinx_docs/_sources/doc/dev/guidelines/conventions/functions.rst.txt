Functions
=========

.. toctree::
   :maxdepth: 2
   :caption: Contents:


A Function Should Perform a Single Logical Operation
----------------------------------------------------

A function that performs a single operation is simpler to understand, test, and
reuse.


Keep Functions Short and Simple
-------------------------------

Large functions are hard to read, more likely to contain complex code, and more
likely to have variables in larger rather than minimal scopes.

Functions with complex control structures are more likely to be long and more
likely to hide logical errors

*"It doesn't fit on a screen"* is often a good practical definition of *"far
too large."*

Break large functions up into smaller cohesive and named functions. Small
simple functions are easily inlined where the cost of a function call is
significant.

If a Function Might Have to be Evaluated at Compile Time, Declare it constexpr
------------------------------------------------------------------------------

``constexpr`` is needed to tell the compiler to allow compile-time evaluation. Adding it e.g. to the
constructor of your type might allow storing objects of your type in ROM.

Note that ``constexpr`` does not guarantee compile-time evaluation; it just guarantees that the
function can be evaluated at compile time for constant expression arguments if the programmer
requires it or the compiler decides to do so to optimize.

The only way to ensure that the function can be evaluated at compile-time, is testing it at
compile-time, by checking its output within a ``static_assert``.

This rule quotes `rule F.4 of the C++ Core Guidelines
<https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#user-content-f4-if-a-function-might-have-to-be-evaluated-at-compile-time-declare-it-constexpr>`_.


For "in" Parameters, Pass Cheaply-Copied Types by Value and Others by Reference to const
----------------------------------------------------------------------------------------

Both let the caller know that a function will not modify the argument, and both
allow initialization by rvalues.

What is "cheap to copy" depends on the machine architecture, but two or three
words (doubles, pointers, references) are usually best passed by value.

When copying is cheap, nothing beats the simplicity and safety of copying, and
for small objects (up to two or three words) it is also faster than passing by
reference because it does not require an extra indirection to access from the
function.

.. code-block:: cpp

    void f1(const string& s);   // pass by reference to const; always cheap
    void f2(string s);          // potentially expensive

    void f3(int x);             // Unbeatable
    void f4(const int& x);      // overhead on access in f4()


Raw Pointer Arguments
---------------------

In general, avoid passing raw pointer arguments, but in cases where pointers
are really needed, they have to be checked before they are dereferenced.

Example:

.. code-block:: cpp

    void f1(BufferType* buffer)
    {
        if (buffer == nullptr)
        {
            return;
        }
        ...
    }

For performance reasons or due to third-party API compatibility this
check can be omitted, but then the missing check must be documented like this:

.. code-block:: cpp

    /**
    * ...
    *
    * \param[in] buffer Description of buffer...
                        Note: buffer will not be checked for null, it has to be
                        ensured that buffer is not null before calling this function.
    */
    void f1(BufferType* buffer)
    ...

Raw Byte Array Arguments
------------------------

Code that uses raw byte arrays and a length are very prone to errors or misuse.

.. code-block:: cpp

    void start(Connection& conn, uint8_t data[], uint16_t length);

It's unclear from the method signature what will happen to data and length.
Look for existing buffer classes and reuse one of those, for example ``etl::array<>`` or
``etl::span<>``.


Translation-Unit Local Functions
--------------------------------

To mark a function as translation-unit local, place the function inside an anonymous namespace. This
is preferred over marking the function as static because the static keyword already performs many
different jobs, making its use potentially ambiguous and/or confusing.

.. code-block:: cpp

    void global();                 // bad, is global

    static void staticallyLocal(); // bad, is translation-unit local, but uses static

    namespace
    {
    void translationUnitLocal();   // good, is translation-unit local in anonymous namespace

    } // namespace


Lambdas
-------

When using lambdas, be careful about object lifetimes: objects captured by reference within a lambda
have to be still in scope at the time of lambda execution. Also lambda objects themselves have to
still exist when they're executed - it might be tricky while using function references like
``etl::delegate``.
