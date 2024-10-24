.. _estd_math:

math
====

Overview
--------

The ``estd::math`` namespace contains mathematical utility functions, including an abs function
template. This function computes the absolute value of a given number, converting it to the
specified return type. The abs function ensures type safety and proper conversion through
compile-time checks, making it suitable for both integral and floating-point types.

Usage
-----
The ReturnType and InputType have to be both integral or both floating point types.
If ReturnType is unsigned or a floating-point, it must be at least the same size as InputType.
If ReturnType is signed and integral, its size must be strictly greater than that of InputType.

Usage Example
-------------

The following example shows the default return type for the abs() function is uint64_t.

.. literalinclude:: ../../examples/math.cpp
   :start-after: EXAMPLE_MATH_ABS_DEFAULT_RETURN_TYPE_START
   :end-before: EXAMPLE_MATH_ABS_DEFAULT_RETURN_TYPE_END
   :language: c++

This example shows that a value of a floating point can have its absolute value be
assigned to a floating-point of the same size.

.. literalinclude:: ../../examples/math.cpp
   :start-after: EXAMPLE_MATH_ABS_SIGNED_MIN_TO_UNSIGNED_START
   :end-before: EXAMPLE_MATH_ABS_SIGNED_MIN_TO_UNSIGNED_END
   :language: c++

Example to show that a value of a floating point can have its absolute value be assigned to a
floating point of the same size.

.. literalinclude:: ../../examples/math.cpp
   :start-after: EXAMPLE_ABS_SIGNED_FLOATING_START
   :end-before: EXAMPLE_ABS_SIGNED_FLOATING_END
   :language: c++
