.. _estd_vec:

estd::vec
=========

Overview
--------

``estd::vec`` is a fixed-capacity contiguous storage sequence container with an
interface similar to the ``std::vector``.

Unlike the STL vector, ``estd::vec`` has an internal fixed-size storage
allocated and will therefore not grow or reallocate memory. This makes its
memory behavior similar to that of the :ref:`estd_array`, but with the added
utility of ``.size()`` tracking the currently used portion of the container.

.. note::
   ``estd::vec`` is a relatively new addition to ``estd``, it was introduced as a replacement for
   :ref:`estd::vector <estd_vector>` to avoid some of its problems. ``estd::vec`` has a limited set
   of operations, which will most likely be extended as we need it.


Usage Context
-------------

.. csv-table::
  :widths: 28,72

  "Preconditions", "Make sure to include the appropriate library header for vec."
  "Assumptions of usage", "The vec has a fixed max_size which is given at the time of declaration. Expanding further than the max_size is not possible."
  "Recommended usage guidance", "Unlike other modules, the declaration and usage of the functions is within the class. Accessing outside the size of vec is not allowed."
