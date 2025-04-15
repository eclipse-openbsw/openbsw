.. _estd_ratio:

ratio
=====

Overview
--------

The ``estd::ratio`` provides compile-time rational arithmetic support.

This template is used to instantiate types that represent a finite rational
number denoted by a numerator and a denominator, which are implemented as
compile-time constants of type intmax_t. |br| The ratio is not represented by an
object of this type but by the type itself, which uses compile-time constant
members to define the ratio. Therefore, ratio can only be used to express
constants and cannot contain any value.

**N** - Numerator. **D** - Denominator. Their absolute values shall be in the
range of representable values of intmax_t, which is the widest signed integer
type, and **D** shall not be zero.

The static constants **num** and **den** are not the same as the template
arguments **N** and **D** if the greatest common divisor among N and D is not
one, as the num and den are the results of dividing N and D by that greatest
common divisor.

Usage
-----

The usage constraints and guidelines apply for ratio:

   .. list-table::
      :widths: 100 70

      *  - Constraints
         - ``ratio`` with zero denominator cannot be created. |br| |br|
           Static assertion is thrown if attempt to create is made.

Example
-------

The ``estd::ratio.h`` provides compile-time rational arithmetic support.

Following example shows the basic functionalities of ratio:

.. literalinclude:: ../../examples/ratio.cpp
   :start-after: EXAMPLE_RATIO_BASIC_FUNCTIONALITY_START
   :end-before: EXAMPLE_RATIO_BASIC_FUNCTIONALITY_END
   :language: c++

Usage example of comparison operator in ratio:

.. literalinclude:: ../../examples/ratio.cpp
   :start-after: EXAMPLE_RATIO_COMPARISON_OPERATORS_START
   :end-before: EXAMPLE_RATIO_COMPARISON_OPERATORS_END
   :language: c++

Usage example of arithmetic operator in ratio:

.. literalinclude:: ../../examples/ratio.cpp
   :start-after: EXAMPLE_RATIO_ARITHMETIC_OPERATORS_START
   :end-before: EXAMPLE_RATIO_ARITHMETIC_OPERATORS_END
   :language: c++

Example of some predefined types in ratio:

.. literalinclude:: ../../examples/ratio.cpp
   :start-after: EXAMPLE_RATIO_PREDEFINED_TYPES_START
   :end-before: EXAMPLE_RATIO_PREDEFINED_TYPES_END
   :language: c++
