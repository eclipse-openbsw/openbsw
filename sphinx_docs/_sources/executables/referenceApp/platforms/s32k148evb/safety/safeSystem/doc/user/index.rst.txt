User Documentation
==================

The ``safeSystem`` module monitors critical voltages, including the **ADC reference** and
**internal supply voltages** (e.g., **3.3V Flash**, **3.3V Oscillator**, **1.2V Core**).
It ensures these voltages remain within safe limits and triggers corrective actions,
such as a software reset, if anomalies are detected. The ``cyclic()`` method is called
every 10 milliseconds to perform this check.

The ``safeSystem`` expects a monitor in the ``SafeSupervisor``, which is responsible for
logging the error and triggering a software system reset in case of a voltage anomaly, in present
implementation ``adcReferenceMonitor`` and ``internalSupplyMonitor`` are used.

Methods in safeSystem
---------------------

`safeSystem` consists of following methods:

.. sourceinclude:: include/safeSystem/SafeSystem.h
    :start-after: METHODS_START
    :end-before: METHODS_END
    :language: c++
    :dedent: 0
