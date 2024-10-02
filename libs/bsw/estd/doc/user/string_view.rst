.. _estd_string_view:

string_view
===========

Overview
--------

``estd::string_view`` is a lightweight alternative to ``estd::string``. It serves as a
non-owning, read-only view of a string. Unlike ``estd::string``, it does not manage the
underlying memory but instead provides a reference to an existing string. ``estd::string_view``
is commonly used when you want to efficiently process and operate on strings without modifying
them. It also offers functions like ``begin()``, ``operator[]``, ``data()``, ``remove_prefix()``
etc.

It also contains a special struct named as ``string_view_tokens`` which works over tokens of a
``string_view`` with given a set of delimiters.

Usage
-----

The usage constraints and guidelines apply for ``string_view``.

.. list-table::
    :widths: 30 70

    * - Usage guidance
      - While using functions like ``operator[]``, ``at()``, ``copy()`` and ``substr()``, if the
        index passed to the functions is outside the bounds will result in a runtime error.
        |br| |br|
        The index ranges for ``estd::string_view`` starts from 0 to size() - 1.

Example
-------

The below code shows how ``operator=()`` assigns values to the string.

.. literalinclude:: ../../examples/string_view.cpp
   :start-after: EXAMPLE_STRING_VIEW_ASSIGN_START
   :end-before: EXAMPLE_STRING_VIEW_ASSIGN_END
   :language: c++
   :dedent: 4

The below code shows the swapping of two views.

.. literalinclude:: ../../examples/string_view.cpp
   :start-after: EXAMPLE_STRING_VIEW_SWAP_START
   :end-before: EXAMPLE_STRING_VIEW_SWAP_END
   :language: c++
   :dedent: 4

The piece of code shows how the ``string_view`` can be used for tokenization.

.. literalinclude:: ../../examples/string_view.cpp
   :start-after: EXAMPLE_STRING_VIEW_TOKENIZE_START
   :end-before: EXAMPLE_STRING_VIEW_TOKENIZE_END
   :language: c++
