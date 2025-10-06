Statements and Expressions
==========================

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Expressions with Booleans
-------------------------

Never use the constants ``true`` or ``false`` as operands to the operators
``==`` or ``!=``. It makes the expressions unnecessarily long and hard to read.

.. code-block:: cpp

    // good
    if (hasSomething && !isGood)
    {
        // ...
    }

    // bad
    if (true == ((true == hasSomething) && (false == isGood)))
    {
        // ...
    }


Trivial if-else
---------------

Just don't do this, you are not paid per lines of code produced.

.. code-block:: cpp

    return isGood;    // good

    if (isGood)       // bad
    {
        return true;
    }
    else
    {
        return false;
    }


Avoid Magic Constants
---------------------

Unnamed constants embedded in expressions are easily overlooked and often hard
to understand.

.. code-block:: cpp

    constexpr int first_month = 1;
    constexpr int last_month  = 12;
    for (int m = first_month; m <= last_month; ++m)   // good, constants are named
    {
        std::cout << month[m] << '\n';
    }

    for (int m = 1; m <= 12; ++m)   // bad, magic constants 1 and 12
    {
        std::cout << month[m] << '\n';
    }


Null Pointer
------------

**Don't use zero integer literal.**

``0`` is a suboptimal way to represent a null pointer constant for multiple reasons. Firstly, it is
an integer literal, which is most commonly used to represent the numerical value zero rather than
an address. Its duality introduces cognitive overhead to source code readers.

.. code-block:: cpp

    int main()
    {
        // Unclear whether foo accepts an integer or address
        something::foo(0);
    }

Another problem lies in overload resolution. Overloading a function accepting int* with one
accepting int might silently change the meaning of existing code:

.. code-block:: cpp

    // before overloading

    void foo(int*);   // (0)
    // void foo(int);    // (1)
    int main()
    {
        foo(0);       // Calls (0)
    }

    // after overloading

    void foo(int*);   // (0)
    void foo(int);    // (1)
    int main()
    {
        foo(0);       // Calls (1)
    }

**Don't use the NULL macro.**

The ``NULL`` macro is defined as "an implementation-defined null pointer constant". This gives
implementations the freedom to implement NULL as any zero integral literal (e.g. ``0``, ``0L``) or
as ``nullptr``. Similarly to the overload resolution situation described above, this definition can
cause problems when new overloads are added to an existing overload set.

Imagine adding a function accepting either ``std::nullptr_t``, ``long``, ``int``, or ``int*`` to an
existing overload set that is being invoked with ``NULL``: the behavior is now hard to predict
and may vary depending on the standard library implementation being used.

Additionally, C and C++ code can interpret NULL differently which leads to incompatibilities.

**Use nullptr!**

Modern code should use ``nullptr`` instead of ``0`` or ``NULL`` to maximize readability and
prevent surprising overload resolution outcomes between pointer and integral types.

.. code-block:: cpp

    void foo(int*);

    int main()
    {
        foo(nullptr);
    }


See also:

- `<http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2048r0.html>`_
- `<https://en.cppreference.com/w/cpp/types/NULL>`_
- `<https://en.cppreference.com/w/cpp/language/nullptr>`_

Order of Evaluation
-------------------

| Do not depend on the order of the evaluation of expressions.
| Examples:

.. code-block:: cpp

    // undefined behaviour, i is evaluated more than once in an unsequenced manner
    uint32_t a = i + b[++i];

    // correct code
    ++i;
    uint32_t a = i + b[i];

.. code-block:: cpp

    // undefined behaviour, the order of evaluation for function arguments is unspecified
    func(a(), b());

    // correct code
    a_val = a();
    b_val = b();
    func(a_val, b_val);

For more information and examples see `CERT rule EXP50 <https://wiki.sei.cmu.edu/confluence/display
/cplusplus/EXP50-CPP.+Do+not+depend+on+the+order+of+evaluation+for+side+effects>`_.

Asserts
-------

To control the behavior of asserts on ECUs, use the ``estd_assert`` macro instead of the regular
``assert`` from the standard *cassert* header.

In contrast to a *PC* program where asserts are ignored in release builds, an ``estd_assert``
will be present in release code causing an ECU reset and most likely a customer noticeable event.
We usually do not distinguish between debug and release builds.

Therefore, an ``estd_assert`` shall only be used if continuing execution is impossible, e.g. because
memory got corrupted (out of bounds access through operator[]). Comment the usage with a
justification.

It might be acceptable during development to use asserts. In this case use ``estd_expect`` instead.
``estd_expect`` behaves the same as ``estd_assert``, but it must be replaced for production with
proper error handling. The usage of ``estd_expect`` is detected by our code analyzers and
presented in the module overview from Cijack.

Summary:

- Prefer proper error handling over asserts
- For temporary asserts, use ``estd_expect``
- For production code asserts, use ``estd_assert`` and comment the usage with a justification

Forward Declarations
--------------------

Use explicit forward declarations for:

- Faster (incremental) builds
- Better testable code

Prefer Using Over typedef for Defining Aliases
----------------------------------------------

With ``using``, the new name comes first rather than being embedded somewhere in a declaration,
which improves readability. Additionally, ``using`` can be used for template aliases, whereas
``typedef`` s can't easily be templates.

.. code-block:: cpp

    using MyArray = ::etl::array<::some::other::Type, 199>; // good - new name comes first

    typedef ::etl::array<::some::other::Type, 199> MyArray; // bad - new name at the end

    template <typename T>
    using ArrayOf5 = ::etl::array<T, 5>; // template alias - impossible with typedef

OS Specific Code
----------------

Try to avoid OS specific code. This makes your software hardly portable and difficult to test. Use
an abstraction layer, e.g. the ``async`` library.

Critical Sections
-----------------

| Avoid critical sections if possible by design. Sometimes this is not feasible, e.g. when accessing
  shared HW resources.
| In these cases use a lock mechanism with the RAII pattern:

.. code-block:: cpp

    ...
    {
        ESR_UNUSED const interrupts::SuspendResumeAllInterruptsScopedLock lock;

        // Critical section here
    }
    ...

Buffers
-------

If buffers are used in the code, ensure that they are protected against under- and overflow.
This can be done implicitly by design or by range checks. If you decide not to implement such a
check, e.g. due to performance reasons, document that behaviour.

A missing check might be unacceptable, e.g. in security or safety relevant code.

Delays
------

It is often hard to understand the values of delays. Explain the chosen values in a comment.

Retries
-------

Document retries in the code or make it obvious by designing a clear API.
