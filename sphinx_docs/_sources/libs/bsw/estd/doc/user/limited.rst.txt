.. _estd_limited:

estd::limited
=============

Overview
--------

``estd::limited`` is intended to replace ``optional`` for integral types. It
uses less memory than ``optional`` at the cost of making one of possible values
forbidden, and thus limited. The semantics of the type are organized in a way
that when the internal representation holds the forbidden value, on the outside
it indicates an absence of any value.

Usage Context
-------------

``limited`` is useful when a function returns a value that is not always
present, but the absence of the value can be represented by a specific value.

In other words, consider the following code:

Example
-------

.. code-block:: c++

    constexpr uint32_t NO_NUMBER = 0xFFFFFFFF;

    uint32_t getNumber()
    {
        if (...)
        {
            return number;
        }

        return NO_NUMBER;
    }

In this example, the caller would have to check the returned value against
``NO_NUMBER`` to get the semantic meaning of it. If we were to use ``optional``,
it would require one additional word of memory. If we know for sure
``0xFFFFFFFF``, will never be used as a value, ``limited`` can be used instead.

.. code-block:: c++

    estd::limited<uint32_t> getNumber()
    {
        if (...)
        {
            return number;
        }

        return {};
    }

The caller then checks the presence of value by calling ``.has_value()``. This
makes code more expressive:

.. code-block:: c++

    const auto number = getNumber();

    if (number.has_value())
    {
        useNumber(number.get());
    }

``limited`` works for integral types and enums. By default, the forbidden value
is chosen equal to ``numeric_limits<T>::max()``. For enum types, ``T`` is the
``underlying_type``.
