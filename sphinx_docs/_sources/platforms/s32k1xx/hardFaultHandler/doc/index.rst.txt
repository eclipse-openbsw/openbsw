hardFaultHandler
================

Overview
--------

HardFault handler does the following things:

1. Recover after ECC error happened during safe-read operation.
2. In case of further fault, save CPU registers to no-init RAM section.
3. Jump to externally defined ``HardFault_Handler_Final()`` to execute user handling.

To have meaningful registers dump, it's crucial not to corrupt CPU registers during the first step.

Warning:
--------

Do not modify ``HardFault_Handler()`` if it's not absolutely necessary. Using local variables or
function calls will lead to stack use which will ruin the logic of saving registers dump.



Registers dump format
---------------------

The following data is saved to the dump:

- CPU registers at the moment of exception
- LR and PSR registers at the entry of exception handler
- stack contents (as much as possible)
- checksum of the dump

The checksum is needed after CPU reset to ensure that the dump contains correct information rather
than random noise.

Detailed dump format can be found in *hardFaultHandler.S* in section "Dump fields offsets".


How to test
-----------

Intentional hard fault might be used for testing, for example by placing incorrect instruction:

.. code:: c++

    asm(".short 0xFFFF");

Behavior of the handler is different for execution from OS and OS-less environment, because
different stack modes are used. To test both cases, a fault should be produced inside any of task
functions as well as before starting OS kernel.