safeSystem
==========

Overview
--------

The ``safeSystem`` module ensures the platform health of the S32K1xx controller by monitoring
critical voltage levels.

It currently focuses on monitoring the **ADC voltage** and **internal supply voltage** to
ensure proper operation. The system ensures that the following voltages remain within safe
operating limits:

- **ADC reference voltage**
- **Internal supply voltages** like the **3.3V Flash**, **3.3V Oscillator**, and **1.2V Core**
  voltages.

If any of these voltages deviate from expected values, the system takes corrective action by
logging the error and triggering a software system reset via the SafeSupervisor.


.. tolerant-toctree::
    :hidden:

    user/index
