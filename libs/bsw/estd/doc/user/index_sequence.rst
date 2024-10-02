.. _estd_index_sequence:

index_sequence
==============

Introduction
------------

The ``index_sequence`` is a compile-time integer sequence to represent indices.

It is employed to facilitate operations on a fixed number of elements, such as accessing
a tuple or array.

``index_sequence`` generates sequences of indices through template recursion and specialization.
It facilitates the generation of an index sequence of a specified length at compile time.

Usage
-----

In addition to the general safety assumptions at the module level which are taken care
at the system, the usage guidelines apply for index_sequence.

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - estd::index_sequence is used to swaping the elements in tuple.
           |br| |br|
           Passing estd::index_sequence as parameter to a function indicates that the
           implementation is tailored to work with a specific sequence of indices.

Example
-------

The index_sequence generates a sequence of indices.

The following example shows the usage of index_sequence to generate sequence of numbers:

.. literalinclude:: ../../examples/index_sequence.cpp
   :start-after: EXAMPLE_INDEX_SEQUENCE_STRUCT_START
   :end-before: EXAMPLE_INDEX_SEQUENCE_STRUCT_END
   :language: c++

.. literalinclude:: ../../examples/index_sequence.cpp
   :start-after: EXAMPLE_INDEX_SEQUENCE_START
   :end-before: EXAMPLE_INDEX_SEQUENCE_END
   :language: c++

The following example shows the usage of index_sequence in tuple:

.. literalinclude:: ../../examples/index_sequence.cpp
   :start-after: EXAMPLE_INDEX_SEQUENCE_IN_TUPLE_START
   :end-before: EXAMPLE_INDEX_SEQUENCE_IN_TUPLE_END
   :language: c++