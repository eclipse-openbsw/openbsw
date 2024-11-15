.. _util_assert:

assert
======

Introduction
------------

In estd, assert is a debugging function defined in the assert.h header, Which is
used to test assumptions and validate that certain conditions hold true during
the execution of a program.

It is a convenient way to catch logical errors and unexpected conditions during
the development and testing phases of a program.

Usage
-----

The usage preconditions and guidelines apply for assert:

Preconditions
+++++++++++++

ESTL_HARD_ASSERT_FUNC
~~~~~~~~~~~~~~~~~~~~~

The flag ``ESTL_HARD_ASSERT_FUNC`` needs to be defined in  ``DefaultToolchain``
in ``project.meta`` file to enable Hardware/Project specific functionality for
assert as shown in below example.

.. code-block::

   DefaultToolchain GCC {
      Compiler CPP, command: "arm-none-eabi-gcc" {
      Define "ESTL_HARD_ASSERT_FUNC=__ecu_assert"
      Define "ESTL_HARD_ASSERT_HEADER=\"common/ecu_assert.h\""
      }
      Compiler C, command: "arm-none-eabi-gcc" {
      Define "ESTL_HARD_ASSERT_FUNC=__ecu_assert"
      Define "ESTL_HARD_ASSERT_HEADER=\"common/ecu_assert.h\""
      }

Note: Failure to define Project specific assert function, will result in
compilation error with warning to define the assertion function.


Example
-------

The following example shows the usage of a ``estd_assert_no_macro``:

.. code-block:: c++

   #include <estd/assert.h>

   // Hypothetical definition of estd_assert_no_macro
   void estd_assert_no_macro(bool condition, char const* message)
   {
      if (!condition)
      {
         // Terminate the program or handle the error appropriately
         abort();
      }
   }

   int main()
   {
      int assert_a = 5;
      int assert_b = 0;

      // This assertion will pass
      estd_assert_no_macro(assert_a == 5, "assert_a should be 5");

      // This assertion will fail and terminate the program
      estd_assert_no_macro(assert_b != 0, "assert_b should not be 0");

      return 0;
   }