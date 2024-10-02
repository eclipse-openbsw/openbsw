.. _estd_none:

none
====

Overview
--------

The ``estd::none`` is designed to provide a uniform representation for a "none"
or "null" value across various data types in C++.

It is designed to use as a place holder for other function and template
parameters. It also defines a templated conversion operator defined as operator
T() const, which represents a type that can be implicitly converted to a
default-constructed instance of any other type.

The ``estd::none`` can only be used as a placeholder and it is not considered as
any value or target.

Usage
-----

The usage guidelines apply for ``estd::none``:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - The ``estd::none`` can be used |br| |br|
           * In places where uninitialized instances are created to avoid initialization to
           random values. |br| |br|
           * As the default value, when default values are not known. |br| |br|
           * As the placeholder to avoid ambiguity. |br| |br|
           * In the case when a function does not return any value.

Examples
--------

``estd::none`` is designed to use as a place holder for other function and
template parameters.

This example gives the usage of assigning estd::none to a uninitialized object.

.. literalinclude:: ../../examples/none.cpp
   :start-after: EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_STRUCT_START
   :end-before: EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_STRUCT_END
   :language: c++

.. literalinclude:: ../../examples/none.cpp
   :start-after: EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_START
   :end-before: EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_END
   :language: c++

This example gives the usage of conversion estd::none conversion operator to a
uninitialized object.

.. literalinclude:: ../../examples/none.cpp
   :start-after: EXAMPLE_NONE_CONVERSION_OPERATOR_OF_NONE_START
   :end-before: EXAMPLE_NONE_CONVERSION_OPERATOR_OF_NONE_END
   :language: c++