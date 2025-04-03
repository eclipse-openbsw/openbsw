.. _estd:

estd - Embedded Standard Library
================================

Overview
--------

The ``estd`` library is the ESR Labs standard library for developing embedded
systems.

The C++ standard library heavily relies on dynamic memory allocation. This is
not acceptable in most embedded systems, effectively preventing us from using a
vast majority of features of the standard library. In embedded software
development, we also struggle with very limited support for newer C++ standards
in the compilers that we have to use.

``estd``'s purpose is to fill those gaps and provide developers at ESR Labs
basic tools for building embedded software in C++. It contains container
classes, implementations of generic algorithms, helpful vocabulary types (like
``optional``, ``variant``, ``result``) and many other utilities.

Some parts of the ``estd`` API are designed to mimic the C++ standard library,
while some are invented at ESR Labs. To facilitate integration and enable usage
of e.g. ``<algorithm>``, the container types available in ``estd`` expose
interfaces compatible with the C++ STL.


User and Design Documentation
-----------------------------

The user documentation helps end-users to understand and effectively use the ``estd`` module.
The design documentation helps developers to  understand the design of the ``estd`` module.

.. toctree::
    :maxdepth: 2
    :glob:

    */index
