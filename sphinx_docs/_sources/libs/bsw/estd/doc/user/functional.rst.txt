.. _estd_functional:

functional
==========

function
--------

Overview
++++++++

``estd::function`` provides a set of predefined class templates for function
objects. |br| |br| ``estd::function`` objects are objects specifically designed
to be used with a syntax similar to that of functions. Instances of
``estd::function`` can store, copy, and invoke any callable target functions,
lambda expressions, bind expressions, or other function objects, as well as
pointer to member functions of a class.

Usage
+++++

The usage guidelines apply for function:

.. list-table::
    :widths: 30 70

    * - Usage guidance
      - The **estd::function** is used as a function pointer hence it should
        always be initialized to appropriate function or **estd::none** to avoid
        undefined behavior.

Example
+++++++

The following example shows a simple example of the usage of a
``estd::function`` with a function:

.. literalinclude:: ../../examples/functional.cpp
   :start-after: EXAMPLE_FUNCTION_ZERO_PARAM_START
   :end-before: EXAMPLE_FUNCTION_ZERO_PARAM_END
   :language: c++

The next example shows a simple example of the usage of a ``estd::function``
with a functor:

.. literalinclude:: ../../examples/functional.cpp
   :start-after: EXAMPLE_FUNCTION_FUNCTOR_ZERO_PARAM_START
   :end-before: EXAMPLE_FUNCTION_FUNCTOR_ZERO_PARAM_END
   :language: c++

The next example shows a simple example of the usage of a ``estd::function``
with a lambda function:

.. literalinclude:: ../../examples/functional.cpp
   :start-after: EXAMPLE_FUNCTION_LAMBDA_PARAM_START
   :end-before: EXAMPLE_FUNCTION_LAMBDA_PARAM_END
   :language: c++
   :dedent: 4

closure
-------

Introduction
++++++++++++

``estd::closure`` provides functions which are used to bind a function or a
member function with arguments. |br| |br| It creates a new function object by
“binding” a function or member function to a specific object or value, allowing
it to be called with a different number of arguments.

Usage
+++++

The usage guidelines apply for closure:

.. list-table::
    :widths: 30 70

    * - Usage guidance
      - The **estd::closure** can be used with functions having maximum of five
        arguments.

Example
+++++++

The following example shows a simple example of the usage of a
``estd::closure`` ``estd::bind_all``:

.. literalinclude:: ../../examples/functional.cpp
   :start-after: EXAMPLE_CLOSURE_THREE_PARAM_START
   :end-before: EXAMPLE_CLOSURE_THREE_PARAM_END
   :language: c++

The next example shows a simple example of the usage of a
``estd::closure`` ``estd::bind1st``:

.. literalinclude:: ../../examples/functional.cpp
   :start-after: EXAMPLE_CLOSURE_BIND1ST_THREE_PARAM_START
   :end-before: EXAMPLE_CLOSURE_BIND1ST_THREE_PARAM_END
   :language: c++