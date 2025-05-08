.. _estd_chrono:

chrono
======

Introduction
------------

The chrono library is a flexible collection of types that track time with varying degrees of
precision. It is modeled after the standard library's `std::chrono`.

It defines three main types (durations, clocks, and time points) as well as utility functions and
common typedefs. The chrono is most suited for measuring time intervals and independent of the
system clock. If the destination type has less precision, the value is truncated.

Usage
-----

In addition to the general safety assumptions at the module level which are taken care
at the system, the usage guidelines apply for ``estd::chrono``.

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Implement function getSystemTimeNs() for full chrono support on the respective platform.

Example
-------

The following example shows that "42 seconds" could be represented by a duration consisting of 42
ticks of a 1-second time unit, or of 42'000 ticks of a 1-millisecond time unit. Kindly refer typedefs for the supported clock units.

.. literalinclude:: ../../examples/chrono.cpp
   :start-after: EXAMPLE_CHRONO_DURATION_START
   :end-before: EXAMPLE_CHRONO_DURATION_END
   :language: c++
   :dedent: 4

Usage example of time point is as follows:

.. literalinclude:: ../../examples/chrono.cpp
   :start-after: EXAMPLE_CHRONO_TIME_POINT_START
   :end-before: EXAMPLE_CHRONO_TIME_POINT_END
   :language: c++
   :dedent: 4
