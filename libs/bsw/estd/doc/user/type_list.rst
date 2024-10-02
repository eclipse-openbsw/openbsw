.. _estd_type_list:

type_list
=========

Overview
--------

``type_list`` is a template meta programming compile-time construct, used for creating and
manipulating collections of types. It provides the fundamental operations of types, like
lists of values support. It is a recursive data structure, which is composed of a head element
and a tail element. The head is a single type, whereas the tail is again a type_list or the
termination marker type_list_end.

Usage
-----

If we consider three types, type_list can be used as below:

.. code:: cpp

    using AliasName = type_list<type1,
                        type_list<type2,
                            type_list<type3>>>;

- Using make_type_list helper:

.. code:: cpp

    using AliasName = make_type_list<type1, type2, type3>::type;

Example
-------

The following example shows how to get the position of the specific type within the type_list:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_TYPE_INDEXING_START
   :end-before: EXAMPLE_TYPE_INDEXING_END
   :language: c++

The below code snippet shows how to call the contains method to check if a type is present in the
type_list:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_TYPE_LIST_CONTAINS_START
   :end-before: EXAMPLE_TYPE_LIST_CONTAINS_END
   :language: c++

The ``max_size()`` method returns the size of greatest type in the type_list, below is the example
on how to use it:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_MAXSIZE_START
   :end-before: EXAMPLE_MAXSIZE_END
   :language: c++

Example to check whether all the types of the type_list are of the type provided as parameter:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_TRAITS_START
   :end-before: EXAMPLE_TRAITS_END
   :language: c++

The ``size()`` method returns the number of types present in type_list, below code is the example
on how to use it:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_SIZE_START
   :end-before: EXAMPLE_SIZE_END
   :language: c++

The ``max_align()`` method returns the maximum alignment requirement among the types in type_list:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_MAX_ALIGN_START
   :end-before: EXAMPLE_MAX_ALIGN_END
   :language: c++

The below code shows how the type and multiple type lists can be combined into single flattened
type list:

.. literalinclude:: ../../examples/type_list.cpp
   :start-after: EXAMPLE_FLAT_TYPE_LIST_START
   :end-before: EXAMPLE_FLAT_TYPE_LIST_END
   :language: c++
