.. _estd_string:

string
======

Overview
--------

``estd::string`` is similar to the standard C++ ``std::basic_string`` class which offers multiple
functions like ``size()``, ``length()``, ``resize()``, ``data()`` etc. It also contains a special
template function named as ``strtonum<T>()`` which converts a ``string`` as a ``slice`` to an
integer of type ``T``, where ``T`` must be a primitive number type.

Usage
-----

The usage constraints and guidelines apply for ``string``.

.. list-table::
      :widths: 30 70

      * - Constraint
        - The ``string`` has a fixed max_size which is given at the time of declaration.
      * - Usage guidance
        - The size of the ``string`` should have a positive integer value.
          |br| |br|
          While accessing the elements using ``at()`` and ``operator[]`` functions, the index
          passed to the function should not be out of bounds, as this will result in a runtime
          error. The index ranges for ``estd::string`` starts from 0 to size() - 1.

Example
-------

The below code shows how ``operator=()`` assigns values to the ``string``.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_ASSIGN_START
   :end-before: EXAMPLE_STRING_ASSIGN_END
   :language: c++
   :dedent: 4

The below code shows how ``append()`` appends characters to the ``string``.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_APPEND_START
   :end-before: EXAMPLE_STRING_APPEND_END
   :language: c++
   :dedent: 4

The below code shows how ``operator+=()`` appends characters to the ``string``.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_APPEND_CHAR_START
   :end-before: EXAMPLE_STRING_APPEND_CHAR_END
   :language: c++
   :dedent: 4

The below code shows how ``resize()`` resize the ``string`` and add values to it.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_RESIZE_START
   :end-before: EXAMPLE_STRING_RESIZE_END
   :language: c++
   :dedent: 4

The below code shows how ``operator==()`` compares two strings.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_EQUAL_START
   :end-before: EXAMPLE_STRING_EQUAL_END
   :language: c++
   :dedent: 4

The below code shows how to get the length, size and max_size of a ``string``.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_LENGTH_START
   :end-before: EXAMPLE_STRING_LENGTH_END
   :language: c++
   :dedent: 4

The below code shows a conversion of an array of chars of a lower case hex number with prefix
"0x" to a uint_8t with base 16.

.. literalinclude:: ../../examples/string.cpp
   :start-after: EXAMPLE_STRING_TO_NUM_START
   :end-before: EXAMPLE_STRING_TO_NUM_END
   :language: c++
   :dedent: 4