.. _estd_va_list_ref:

va_list_ref
===========

Overview
--------

``va_list_ref`` is a helper class to store a reference to a ``std::va_list``
starting with a call of macro va_start.

In C++98, there's the lack of the ``std::va_copy`` macro and thus only
references to ``std::va_list`` types can be used. Unfortunately the handling of
these references is platform- specific. Therefore more than one implementation
is provided and va_list_ref is a typedef.

``std::va_list`` is a complete object type (in practice, a unique built-in type
or char*) suitable for holding the information needed by the macros
``std::va_start`` (enables access), ``std::va_copy`` (makes a copy),
``std::va_arg`` (accesses the next argument), and ``std::va_end`` (ends
traversal). These objects contains the pointer to the argument list which gets
passed to variadic functions (functions that can take a variable number of
arguments).

Example
-------

This example shows how to create va_list_ref object as reference to variable :

.. literalinclude:: ../../examples/va_list_ref.cpp
   :start-after: EXAMPLE_VA_LIST_REF_VARIABLE_START
   :end-before: EXAMPLE_VA_LIST_REF_VARIABLE_END
   :language: c++

This example shows how to create va_list_ref object as reference to array :

.. literalinclude:: ../../examples/va_list_ref.cpp
   :start-after: EXAMPLE_VA_LIST_REF_ARRAY_START
   :end-before: EXAMPLE_VA_LIST_REF_ARRAY_END
   :language: c++

This example shows how to create va_list_ref object to std::va_list as reference
to std::va_list :

.. literalinclude:: ../../examples/va_list_ref.cpp
   :start-after: EXAMPLE_VA_LIST_REF_CONSTRUCTOR_START
   :end-before: EXAMPLE_VA_LIST_REF_CONSTRUCTOR_END
   :language: c++

.. literalinclude:: ../../examples/va_list_ref.cpp
   :start-after: EXAMPLE_VA_LIST_REF_CONSTRUCTOR_CALL_START
   :end-before: EXAMPLE_VA_LIST_REF_CONSTRUCTOR_CALL_END
   :language: c++
