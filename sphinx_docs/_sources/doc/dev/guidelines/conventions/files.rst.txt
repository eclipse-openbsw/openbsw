Files
=====

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Including Header Files
----------------------

:rule:`FILE-000` The style used to specify an include shall reflect the locality of the included
header file.

- All includes which are part of the same module shall use quotes ``""``.
- All includes which are not part of the same module (including system headers) shall use angle
  brackets ``<>``.

Standard Headers
----------------

- See `C++ Standard Library headers <https://en.cppreference.com/w/cpp/header>`_ for details.
  Note that not all includes make sense in embedded code. Some parts are replaced by the
  ``etl`` library.
- :rule:`FILE-010` If a file needs to be compiled for C++ and C, use the C headers.
