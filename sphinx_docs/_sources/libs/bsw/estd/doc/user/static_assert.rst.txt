.. _estd_static_assert:

static_assert
=============

Overview
--------

``static_assert`` provides a way to check if a condition is true at compile-time, if false,
compilation stops with error message. ``ESTD_STATIC_ASSERT`` and  ``ESTD_STATIC_ASSERT_MESSAGE``
are the functions defined in the static_assert.h header, which perform static assertion and
depending on the condition, they display fixed and user-defined error messages respectively.

Usage
-----

The usage constraint applicable for static_assert is provided below:

.. list-table::
    :widths: 30 70

    *  - Constraint
       - The static_assert is only a compilation check and will not impact run time.

Example
-------

The following code shows a simple example of the usage of ``ESTD_STATIC_ASSERT``:

.. literalinclude:: ../../examples/static_assert.cpp
   :start-after: EXAMPLE_ESTD_STATIC_ASSERT_START
   :end-before: EXAMPLE_ESTD_STATIC_ASSERT_END
   :language: c++

The below code shows a simple example of the usage of ``ESTD_STATIC_ASSERT_MESSAGE``:

.. literalinclude:: ../../examples/static_assert.cpp
   :start-after: EXAMPLE_ESTD_STATIC_ASSERT_MESSAGE_START
   :end-before: EXAMPLE_ESTD_STATIC_ASSERT_MESSAGE_END
   :language: c++