Memory Protection
=================

This module uses the MPU to protect the safety relevant data. The memory is divided into 5 parts,
The region descriptors from 1 to 5 are used to configure these memory regions. The region descriptor
2 defines the data memory area in RAM(Protected-RAM) and region descriptor 4 defines the safety task
stack. The write access is disabled for Protected-RAM and enabled for safety task stack initially.
And, the other regions defined by MPU region descriptors 1,3 and 5 are given full access.
The linker links all safety relevant symbols into the section mpu_bss. Preinitialized variables
are linked into the mpu_data section. Safety relevant modules shall not contain preinitialized
variables. The linker aborts the linking process if mpu_data section is not empty.

Public API
----------

The public API of `MemoryProtection` consists of following methods:

.. sourceinclude:: include/safeMemory/MemoryProtection.h
    :start-after: PUBLIC_API_START
    :end-before: PUBLIC_API_END
    :language: c++
    :dedent: 0
