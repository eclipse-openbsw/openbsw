.. _estd_uncopyable:

uncopyable
==========

Overview
--------

``estd::uncopyable`` is a utility that can be used to make a class uncopyable.
Contains uncopyable class and macro. The macro can be used to make a class
uncopyable that cannot inherit from the uncopyable base class.

Usage
-----

The usage guideline for uncopyable is provided below:

.. list-table::
    :widths: 30 70

    *  - Usage guidance
       - A class can be made uncopyable using two ways either by using the macro or by inheriting
         from the uncopyable class. |br| |br|
         The class should be made uncopyable if it is not intended to be copied or assigned to.

Example
-------

The following example shows how to make a class uncopyable using macro:

.. sourceinclude:: examples/uncopyable.cpp
    :start-after: EXAMPLE_BEGIN uncopyable macro
    :end-before: EXAMPLE_END uncopyable macro
    :language: c++

The below code shows how to make a class uncopyable by inheriting from uncopyable:

.. sourceinclude:: examples/uncopyable.cpp
    :start-after: EXAMPLE_BEGIN uncopyable class
    :end-before: EXAMPLE_END uncopyable class
    :language: c++