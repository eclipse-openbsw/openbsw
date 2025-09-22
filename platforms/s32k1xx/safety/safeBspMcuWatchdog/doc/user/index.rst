User Documentation
==================

The ``Watchdog`` class provides the S32K1xx Watchdog API.

Public API
----------

The public API of ``Watchdog`` class consists of following constructors and methods:

.. sourceinclude:: include/watchdog/Watchdog.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

The ``startFastTestLow`` method starts the fast testing of low byte of the watchdog counter,
The watchdog timeout value is set in the TOVAL register. The low byte of the counter is selected
for testing, by assigning 10b to the control and status register test bits i.e CS[TST] = 10b.

The ``startFastTestHigh`` method starts the fast testing of high byte of the watchdog counter,
The watchdog timeout value is set in the TOVAL register. The high byte of the counter is selected
for testing, by assigning 11b to the control and status register test bits i.e CS[TST] = 11b.

Fast Testing of Watchdog
------------------------

The fast test is implemented according to the safety reference manual
(Chapter 5.6.9.2: Fast testing of the watchdog, S32K1xx Series Safety Manual, Rev. 7.1, 12/2021).
This means the high and low byte are tested separately, one after the other.
