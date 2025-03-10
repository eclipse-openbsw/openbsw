.. _estd_variant:

estd::variant
=============

Overview
--------

``estd::variant`` represents a type-safe union similar to ``std::variant``. It
holds a value of exactly one of its alternative types.

The size of the variant equals that of its largest alternative plus one size_t
to store the index of the alternative.

Usage Context
-------------

.. list-table::
   :widths: 35 65

   * - Preconditions
     - Make sure to include the appropriate library header for variant.
   * - Assumptions of usage
     - ``estd::variant`` can be used hold a value of exactly one of its alternative types.
   * - Recommended usage guidance
     - ``estd::variant`` can be used in situation where a program need a type-safe union of fixed types, allowing to work with heterogeneous data in more structured manner.
