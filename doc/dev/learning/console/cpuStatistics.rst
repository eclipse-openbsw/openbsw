.. _cpu_statistics:

CPU Statistics
==============

The CPU statistics is used to monitor the CPU usage and to identify bottlenecks in the system.

Statistics Command
------------------

The ``stats`` command used to print the CPU statistics.

-  **stats**    - lifecycle statistics command
    - cpu       - prints CPU statistics
    - stack     - prints stack statistics
    - all       - prints all statistics

Below is the example usage:

.. code-block:: bash

    > stats cpu

    task                    %       total   runs       avg       min       max
    ---------------------------------------------------------------------------
    idle              73.17 %     75869 ms   8381   9052 us    137 us 236821 us
    background         0.85 %       891 ms    167   5339 us     96 us  15522 us
    bsp                0.72 %       749 ms    144   5203 us    107 us  13716 us
    uds                1.60 %      1665 ms    377   4418 us     79 us 141240 us
    demo               1.15 %      1202 ms    331   3633 us     79 us  14758 us
    can                9.12 %      9464 ms   4746   1994 us     71 us  16620 us
    sysadmin           0.94 %       981 ms    300   3270 us     84 us  13446 us
    timer             11.99 %     12432 ms   5082   2446 us     53 us 175031 us

    isr group               %       total   runs       avg       min       max
    ---------------------------------------------------------------------------
    test               0.00 %         0 ms      0      0 us      0 us      0 us

    measurement time: 103680047 us
    ok

Next: :ref:`logger_console`
