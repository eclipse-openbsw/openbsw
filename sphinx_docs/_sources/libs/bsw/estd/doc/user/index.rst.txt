User Documentation
==================

Overview
--------

This documentation helps end-users to understand and effectively use the
``estd`` module. It provides an overview of the module's contents, grouped by
topic, and includes detailed descriptions of the components, their
functionality, and usage examples.

Container
---------

.. uml::

   component estd
   interface array.h
   interface forward_list.h
   estd .up.> [platform] : uses
   estd -- array.h
   estd -- forward_list.h
   estd -- object_pool.h
   estd -- slice.h

Below you will find an overview of the contents of the library, grouped by topic.

Containers
----------

.. toctree::
   :hidden:

   array
   deque
   forward_list
   object_pool
   ratio
   priority_queue
   ring
   math
   stack
   string
   string_view
   tiny_ring
   ordered_map
   ordered_vector
   vec
   vector

.. csv-table::
   :widths: 20,100

   :ref:`array <estd_array>`, "Fixed-size array"
   :ref:`deque <estd_deque>`, "Double-ended queue, for fast insert and remove operation from front and back."
   :ref:`forward_list <estd_forward_list>`, "Forward-linked and intrusive list, for fast and easy insertion."
   :ref:`object_pool <estd_object_pool>`, "Provides a pool of objects of the same type."
   :ref:`ordered_map <estd_ordered_map>`, "Key-value map, based on a sorted vector."
   :ref:`ordered_vector <estd_ordered_vector>`, "Fixed-size ordered vector."
   :ref:`priority_queue <estd_priority_queue>`, "std::priority_queue-like class; keeps its elements
   sorted according to a comparator."
   :ref:`math <estd_math>`, "Provides various mathematical functions and utilities."
   "queue", "First-in, first-out (FIFO) data structure."
   :ref:`stack <estd_stack>`, "Last-in, first-out (LIFO) data structure."
   :ref:`string <estd_string>`, "std::basic_string-like class."
   :ref:`string_view <estd_string_view>`, "std::basic_string_view-like class."
   :ref:`ring <estd_ring>`, "First-in, first-out (FIFO) data structure, implemented as a circular buffer."
   :ref:`tiny_ring <estd_tiny_ring>`, "Variant of the ring buffer, optimized for <256 elements."
   :ref:`vec <estd_vec>`, "Variable length array, with a statically-defined maximum size."
   :ref:`vector <estd_vector>`, "Variable length array, with a statically-defined maximum size."

Vocabulary types
----------------

.. toctree::
   :hidden:

   bitset
   functional
   limited
   multi_slice
   optional
   result
   slice
   variant

.. csv-table::
   :widths: 20,100

   :ref:`bitset <estd_bitset>`, "Represents fixed number of bits."
   :ref:`functional <estd_functional>`, "Function reference to a global or member function."
   :ref:`limited <estd_limited>`, "Checks if integral types or enums have a value or not."
   :ref:`multi_slice <estd_multi_slice>`, "Pointer to multiple same-sized arrays of different types."
   :ref:`optional <estd_optional>`, "Manages an *optional* value: a value that may or may not be present."
   :ref:`result <estd_result>`, "Represents result of an operation, storing a value or an error."
   :ref:`slice <estd_slice>`, "Pointer to a contiguous range of objects; std::span-like class."
   :ref:`variant <estd_variant>`, "Type-safe union."

Type traits, type utils
-----------------------

.. toctree::
   :hidden:

   type_traits
   type_utils
   va_list_ref
   uncopyable

.. csv-table::
   :widths: 20,100

   :ref:`va_list_ref <estd_va_list_ref>`, "Wrapper allowing storage of a reference to a va_list."
   :ref:`type_traits <estd_type_traits>`, "Helper templates, querying properties of types."
   :ref:`type_utils <estd_type_utils>`, "Utility functions wrapping reinterpret_cast."
   :ref:`uncopyable <estd_uncopyable>`, "A macro to make a class uncopyable that cannot inherit from the uncopyable base class."

Other utilities
---------------

.. toctree::
   :hidden:

   assert
   algorithm
   iterator
   memory
   typed_mem
   static_assert
   big_endian
   constructor
   limits
   index_sequence
   little_endian
   indestructible
   singleton
   tuple
   chrono
   type_list
   none
   va_list_ref

.. csv-table::
   :widths: 20,100

   "alloc", "Helper functions for APIs allowing custom memory allocators."
   :ref:`assert <estd_assert>`, "Checking assertions at runtime."
   :ref:`memory <estd_memory>`, "Utility functions which operate on slices."
   :ref:`typed_mem <estd_typed_mem>`, "Typed memory for delayed creation of objects."
   :ref:`big_endian <estd_big_endian>`, "Big-endian encoding (data serialization)."
   :ref:`constructor <estd_constructor>`, "Helper struct providing a way to construct objects in pre-allocated memory (‘placement new’ wrapper)."
   :ref:`index_sequence <estd_index_sequence>`, "Compile-time integer sequence to represent a sequence of indices."
   :ref:`static_assert <estd_static_assert>`, "Checking assertions at compile time."
   :ref:`singleton<estd_singleton>`, "Templated implementation of the singleton pattern."
   :ref:`little_endian <estd_little_endian>`, "Little endian encoding (data serialization)."
   :ref:`chrono <estd_chrono>`, "Time unit types and conversions between them."
   :ref:`indestructible <estd_indestructible>`, "Provides transparent wrapper for global objects that are never destructed."
   :ref:`tuple<estd_tuple>`,"Helper functions to use with the std::tuple type."
   :ref:`type_list <estd_type_list>`, "The compile-time construct for collection of types."
   :ref:`none <estd_none>`, "Helper struct which can be cast to any type, calling the default constructor."
   :ref:`va_list_ref <estd_va_list_ref>`, "Wrapper allowing storage of a reference to a va_list."

Algorithms
----------

.. csv-table::
   :widths: 20,100

   :ref:`iterator <estd_iterator>`, "Helper function operating on iterators, providing C++11 functionality to C++03."
   :ref:`algorithm <estd_algorithm>`, "Generic implementations of commonly-used algorithms."

Numerics
--------

.. csv-table::
   :widths: 20,100

   :ref:`limits <estd_limits>`,"Define numerical limits."
   :ref:`ratio <estd_ratio>`, "std::ratio-like class template compile-time rational arithmetic support."
