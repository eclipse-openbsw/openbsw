.. _estd_result:

estd::result
============

Overview
--------

``estd::result`` is a type used for returning and propagating errors. Similar
concepts exist in many other programming languages and implementations; so the
semantics are similar to:

* ``Result<T, E>`` in Rust
* Boost.Outcome
* P0323R3 ``std::expected``
* ``Either`` in Haskell, Scala and other FP languages

In ``estd::result<T, E>``, ``T`` is the type of value returned on success, and
``E`` is the error type, for example, an error code enum.

.. literalinclude:: ../../examples/result.cpp
   :start-after: EXAMPLE_START return_from_function
   :end-before: EXAMPLE_END return_from_function
   :language: c++


There is an edge case where memory consumption of ``result`` can be optimized,
and that is when ``T`` is ``void`` and ``E`` is an enum. It is possible to use
:ref:`limited <estd_limited>` as internal storage, which typically results in
saving one word of memory. To do so, add a specialization of ``result_traits``
next to enum type definition:

.. literalinclude:: ../../examples/result.cpp
   :start-after: EXAMPLE_START trait
   :end-before: EXAMPLE_END trait
   :language: c++


where ``WhatWentWrong`` is the error type used with ``result<void, E>``.
