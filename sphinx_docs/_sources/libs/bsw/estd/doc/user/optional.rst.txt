.. _estd_optional:

estd::optional
==============

Overview
--------

``estd::optional`` is a wrapper representing a value that may or not be valid,
that adds an additional possible value to the wrapped type. This can be used to
represent an invalid or empty state.

The instance of the wrapped type is stored within the optional itself, alongside
a member that indicates the validity state of this optional.

Unlike the ``std::optional``, taking a ``estd::optional`` of a reference (i.e.
estd::optional<T&>) is allowed and intended as safer alternative to raw
pointers. It implements rebinding semantics. That is, after assigning to an
optional reference, it references the right-hand-side value of the assignment.
Unlike dereferencing a nullptr, trying to access an empty optional will assert
immediately and not cause undefined behavior.

For integral types that have a value that will never be used, ``estd::limited``
may be used instead which exposes an interface similar to optional. The special
forbidden value is then used to indicate the invalid state. This eliminates the
memory overhead of the dedicated validity state member that optional has.

Usage
-----

The usage guidelines apply for 'optional:

.. list-table::
      :widths: 30 70

      *  - Usage guidance
         - optional instance should be checked if it is empty or not before accessing the elements.
           |br| |br|
           Accessing the empty optional instance will result in an assertion.

Example
-------

``estd::optional`` is a wrapper that adds an additional possible value to the
wrapped type. optional intended as safer alternative to raw pointers.

The following example shows creation of optional instance:

.. literalinclude:: ../../examples/optional.cpp
   :start-after: EXAMPLE_OPTIONAL_CREATION_START
   :end-before: EXAMPLE_OPTIONAL_CREATION_END
   :language: c++

The following example shows dereferencing the value using operator->() and
operator*():

.. literalinclude:: ../../examples/optional.cpp
   :start-after: EXAMPLE_OPTIONAL_DEREFERENCING_START
   :end-before: EXAMPLE_OPTIONAL_DEREFERENCING_END
   :language: c++

The following example shows usage of ‘value_or()’ function:

.. literalinclude:: ../../examples/optional.cpp
   :start-after: EXAMPLE_OPTIONAL_VALUE_OR_START
   :end-before: EXAMPLE_OPTIONAL_VALUE_OR_END
   :language: c++

The following example shows usage of comparison operators in optional:

.. literalinclude:: ../../examples/optional.cpp
   :start-after: EXAMPLE_OPTIONAL_COMPARISION_OPERATORS_START
   :end-before: EXAMPLE_OPTIONAL_COMPARISION_OPERATORS_END
   :language: c++
