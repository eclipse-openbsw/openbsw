bspInterruptsImpl
=================


Module Overview
---------------

The module ``bspInterruptsImpl`` implements the functions declared in ``bspInterrupts``.

``bspInterruptsImpl`` manages interrupt-related operations, including the suspension and resumption
of interrupts. When suspending interrupts, it halts all interrupt processes and retrieves the
previous interrupt status. Later, it resumes the previously suspended interrupts.