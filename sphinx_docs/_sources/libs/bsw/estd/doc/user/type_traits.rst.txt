.. _estd_type_traits:

type_traits
===========

Introduction
------------

The type_traits define compile-time template based interfaces to query the
properties of types. The type_traits provides template functions and template
structs that gives the ability to inspect and transform the properties of types.
|br|

For example, considering a generic type **T**, which may be an integer, boolean,
or any other type, With type traits it is possible to query the compiler about
the nature of **T**, such as determining if it is an integer or a pointer, among
other properties. This functionality is particularly beneficial in **conditional
compilation**, allowing the compiler to select the appropriate execution path
based on the input type. |br|

The type_traits can also apply some transformation to a type. For example, given
**T**, you can add/remove the const specifier, the reference or the pointer and
many other transformations.

Usage
-----

The usage constraints and guidelines apply for type_traits:

.. list-table::
    :widths: 30 70

    * - Constraints
      - Apply type_traits only for the compatible type.
    * - Usage guidance
      - This is designed for scenarios where decisions are made to perform actions based on the
        properties of types during compilation. |br| |br|
        Use type traits to check properties of types before applying operations. |br| |br|
        Use type traits whenever there is a need in transformation of one type to another type.

Example
-------

The following examples provides an overview of usage of different type traits:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_SUPPORT_START
   :end-before: EXAMPLE_TYPE_TRAITS_SUPPORT_END
   :language: c++

The following code shows that estd::is_class provides the member constant value
which is equal to true, if T is a class type (but not union). Otherwise, value
is equal to false:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_IS_CLASS_START
   :end-before: EXAMPLE_TYPE_TRAITS_IS_CLASS_END
   :language: c++
   :dedent: 4

The next piece of code shows that is_callable checks whether the given type is
callable or not:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_IS_CALLABLE_START
   :end-before: EXAMPLE_TYPE_TRAITS_IS_CALLABLE__END
   :language: c++
   :dedent: 4

The following code shows that estd::is_signed provides the member constant value
which is equal to true for the floating-point types and the signed integer
types. Otherwise, false for the unsigned integer types and the type bool:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_IS_SIGNED_START
   :end-before: EXAMPLE_TYPE_TRAITS_IS_SIGNED_END
   :language: c++
   :dedent: 4

The next piece of code shows that estd::is_base_of checks if a class is derived
from another:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_IS_BASE_OF_START
   :end-before: EXAMPLE_TYPE_TRAITS_IS_BASE_OF_END
   :language: c++
   :dedent: 4

The following code shows that make_signed makes the given integral type signed:

.. literalinclude:: ../../examples/type_traits.cpp
   :start-after: EXAMPLE_TYPE_TRAITS_MAKE_SIGNED_START
   :end-before: EXAMPLE_TYPE_TRAITS_MAKE_SIGNED_END
   :language: c++
   :dedent: 4