.. _shed:

shed - Relational Data Structures
=================================

Overview
--------

Shed is an embeddable data structures library to provide an alternative to OOP style data modeling
in embedded projects.

This approach is sometimes known as "Entity Component System" architecture and is conceptually
similar to column oriented relational SQL databases. Instead of organizing data as members of
structures or classes, ECS relies mostly on simple/primitives data types that are related to each
other by the concept of an "Entity", which is a common id to find related data.

The main motivations to use this approach:

Easier to optimize memory usage:
Due to the utilization of homogeneous arrays of simple types, it is possible to use different types
of memory for individual columns. For example read-only data can be kept in ROM and there is no
need to copy it to RAM during initialization just to establish a conceptual relation, like it would
be needed in OOP style applications.
Also columns make it easy to calculate the memory usage for specific parts of the application and
see opportunities to reduce it.

Easier path to concurrency:
All operations on column data explicitly declare all data they are accessing and modifying. For this
reason potential concurrent execution paths can easily be identified, in principle even
automatically.

Easier testability:
By moving access restriction away from the data and instead making it an attribute of operations,
there is no "private state" hidden inside objects that is hard to stimulate and verify in test cases.
Also it has been shown that using columns/data as an interface instead of functions/methods on
objects, callbacks as an integration aid can almost always be avoided, reducing or eliminating
the need for complex mocking in test cases.

