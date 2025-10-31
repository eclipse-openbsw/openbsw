Known Issues clang-format 17.x
==============================

.. _clang_format_remove_parentheses:

Removing Parentheses
--------------------

Multiple parentheses as well as parentheses in return statements should be avoided.

.. code-block:: C++

    // good
    convertBitCounting(copyByteLength * CHAR_BIT);

    // bad
    convertBitCounting((copyByteLength * CHAR_BIT));

.. code-block:: C++

    // good
    return *least_significant_address == 0x01;
    return _subnetOverride.getMask() != OVERRIDE_INACTIVE_MASK;

    // bad
    return (*least_significant_address == 0x01);
    return (_subnetOverride.getMask() != OVERRIDE_INACTIVE_MASK);

Setting the clang-format option ``RemoveParentheses`` to ``ReturnStatement`` would remove these
parentheses, but it breaks our code in too many places, so this option **is not enabled yet**.
Please note a warning in the clang-format documentation:

    *Setting this option to any value other than Leave could lead to incorrect code formatting due
    to clang-format’s lack of complete semantic information. As such, extra care should be taken to
    review code changes made by this option.*


