.. _estd_vector:

estd::vector
============

Overview
--------

``estd::vector`` is a fixed-capacity contiguous storage sequence container with
an interface similar to the ``std::vector``.

Unlike the STL vector, ``estd::vector`` has an internal fixed-size storage
allocated and will therefore not grow or reallocate memory. This makes its
memory behavior similar to that of the :ref:`estd_array`, but with the added
utility of ``.size()`` tracking the currently used portion of the container.

Random access has constant time complexity. Arbitrary insertions or deletions
have linear time complexity in the distance to the end of the vector and
invalidate the iterators of elements behind the target position.

Usage Context
-------------
.. csv-table::
  :widths: 28,72

  "Preconditions", "Make sure to include the appropriate library header for vector."
	"Assumptions of usage", "The vector has a fixed max_size which is given at the time of declaration. This memory is allocated in stack. Expanding further than the max_size is not possible."
	"Recommended usage guidance", "Since the declare namespace allocates the memory, use it to create the vector. For handling the vector like passing it to function,  use estd::vector<T, size_type> class. Accessing outside the size of vector is not allowed."
