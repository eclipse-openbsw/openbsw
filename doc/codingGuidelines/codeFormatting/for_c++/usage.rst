clang-format Usage
==================

Usage
-----

Formatting a file:

.. code-block:: bash

    clang-format -style=file -i <file_to_check>

Formatting several files at once:

.. code-block:: bash

    find util -iname *.h -o -iname *.cpp | xargs clang-format -style=file -i

*-i* edits the files in-place.

For editor integration see :ref:`automatic_formatting`.

.. _disable_clang_format_code:

Disable Checks
--------------

Some code sections like tables may look better when manually formatted.

Exclude a piece of code from automatic formatting
with the following comment lines:

.. code-block:: cpp

    // clang-format off
    ...
    code which should stay as it is
    ...
    // clang-format on

