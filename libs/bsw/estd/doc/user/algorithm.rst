.. _estd_algorithm:

algorithm
=========

Overview
--------

The ``estd::algorithm`` provides a set of well-optimized and tested functions
that can be used to perform a variety of operations on ranges of elements. It
can be used with containers such as ``estd::array``, ``estd::vector``,
``estd::deque``, etc., across ``::estd``.

Some of the functions provided by ``estd::algorithm`` include ``min()``,
``max()``, ``swap()``, ``equals()``, ``not_equals()``, ``greater1()``,
``less1()``, ``all_of()``, ``any_of(),`` and ``none_of()``. The algorithm uses
iterators to iterate through these range of elements in the container. The
functions ``min()``, ``max()``, and ``swap()`` differ from other algorithms
because they do not use any iterators or containers, but instead operate
directly using values passed to them.

``estd::UnaryPredicate`` is used in ``estd::algorithm`` to check conditions and
perform operations based on those conditions. It takes one argument and returns
a boolean value.

The ``estd::algorithm`` contains algorithm, which is not part of ``C++98``, is
written for compatibility with ``C++03``. The `estd` algorithm uses iterators
for the access to containers. Kindly refer the respective container for the
element access and iterators.


Usage
-----

The usage constraints and guidelines apply for algorithm:

   .. list-table::
      :widths: 30 70

      *  - Constraints
         - The containers used in algorithm should support iterators.
      *  - Usage guidance
         - Valid arguments/iterators should be passed to the function. |br|
           Usage of empty containers should be avoided.

Example
-------

Here are the examples to create container using different estd containers.

.. literalinclude:: ../../examples/algorithm.cpp
   :start-after: EXAMPLE_START container
   :end-before: EXAMPLE_END container
   :language: c++
   :dedent: 4
