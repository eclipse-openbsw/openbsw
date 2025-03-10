.. _platform_estdint:

estdint
=======

Overview
--------

The header ``estdint.h`` addresses compatibility across different platforms for standard integer
types. It is inspired from the standard ``stdint.h`` header file, which is included in C99 or C++11
standards but is not available for C++98.

Usage
-----

If you include ``<platform/estdint.h>`` this will provide on all supported platforms the same types
and constants to your program as if you were including
`stdint.h <https://en.cppreference.com/w/cpp/header/cstdint>`_ and
`stddef.h <https://en.cppreference.com/w/cpp/header/cstddef>`_ on a C++11
environment, so that all types including ``size_t`` are available.

Strictly speaking it's an error to use ``size_t`` instead of ``std::size_t``. However, most
compilers allow that and our code base heavily relies on the availability of ``size_t`` in the
global namespace. ``estdint.h`` is the place to make this possible if a given platform would not
support this.

Supported Platforms
-------------------

Currently the below mentioned platforms are supported:

* GCC
* Clang
* ARM Keil
* Windriver Diab
* Greenhills
* MSVS
* Tasking
