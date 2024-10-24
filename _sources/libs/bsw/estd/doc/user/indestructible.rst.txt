.. _estd_indestructible:

indestructible
==============

Overview
--------

``estd::indestructible`` provides transparent wrapper for global objects that
are never destructed, which means encapsulating global objects within a class
that ensures these objects persist throughout the lifetime of the program and
are never explicitly destroyed during the runtime of the program.

This is utilized in situations requiring a *global* or *singleton* like
object that remains constant throughout the program's life. Such objects are not
meant to be dynamically created or destroyed.

Usage
-----

Use ``estd::indestructible`` when you need global objects that should not be
explicitly destroyed.

Example
-------

The example shows the usage of Immutable class to create a global object
that is never destructed.

.. literalinclude:: ../../examples/indestructible.cpp
   :start-after: EXAMPLE_INDESTRUCTIBLE_START
   :end-before: EXAMPLE_INDESTRUCTIBLE_END
   :language: c++