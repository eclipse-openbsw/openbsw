.. _estd_type_utils:

type_utils
==========

Overview
--------

A utility class to encapsulate the cast to a single location. It serves as a
utility for working with pointers, raw memory, and type casting in a safe and
generic manner.

Type type_utils offers a comprehensive set of functions for handling type
conversions. These functions improve clarity of code.

Usage
-----

The usage guidelines and pre-condition apply for type_utils:

.. list-table::
    :widths: 30 70

    * - Usage guidance
      - **Contiguous Memory** |br| The cast_to_type and cast_const_to_type functions assume that
        the raw data pointers (uint8_t*) point to continuous memory, it could vector, deque, array
        of type T(underlying memory is continuous and equally spaced of size of one object of given
        type T). The optional index parameter (idx) allows access to a specific object in the
        array. |br| |br| **Destruction** |br| The destroy function assumes that it is safe to call
        the destructor of an object using placement new and explicit destructor invocation."
    * - Pre-Condition
      - **Pointer Initialization** |br| Raw data pointers (uint8_t*) must be properly initialized
        and should point to valid memory locations."

Examples
--------

The following example shows the usage of cast_to_type() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN cast to type
    :end-before: EXAMPLE_END cast to type
    :language: c++
    :dedent: 4

The next piece of code shows the usage of cast_to_raw() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN cast to raw
    :end-before: EXAMPLE_END cast to raw
    :language: c++
    :dedent: 4

The following example shows the usage of cast_const_to_type() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN cast const to type
    :end-before: EXAMPLE_END cast const to type
    :language: c++
    :dedent: 4

The next piece of code shows the usage of cast_from_void() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN cast from void
    :end-before: EXAMPLE_END cast from void
    :language: c++
    :dedent: 4

The following example shows the usage of const_cast_from_void() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN const cast from void
    :end-before: EXAMPLE_END const cast from void
    :language: c++
    :dedent: 4

The next piece of code shows the usage of cast_to_void() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN cast to void
    :end-before: EXAMPLE_END cast to void
    :language: c++
    :dedent: 4

The following example shows the usage of const_cast_to_void() function:

.. sourceinclude:: examples/type_utils.cpp
    :start-after: EXAMPLE_BEGIN const cast to void
    :end-before: EXAMPLE_END const cast to void
    :language: c++
    :dedent: 4