ROM Check
=========

The ROM check feature ensures memory integrity by performing a runtime CRC calculation using the
CRC module over the memory regions defined between ``__checksum_list_start`` and
``__checksum_list_end`` in the linker script. These markers must be added by the user, and this
process can serve as a blueprint for custom projects. Safety-critical data must be placed within
these sections. During runtime, the ``RomCheck`` calculates a CRC value over these regions and
compares it with the CRC calculated during the post-build process of ELF generation.

.. note::
    The runtime ROM check is performed when the system is in an idle state, minimizing the
    impact on system performance and not increasing startup time.

Prerequisites
-------------

To perform post-build CRC calculation and injection, the following requirements must be met:

* **Python**: Ensure that you are using Python version 3.10 or higher.
* **pyelftools==0.32**: pyelftools is a library for parsing and analyzing ELF files.

.. note::

    These requirements are also mentioned in the `requirements.txt` file located in the `safety`
    directory under `tools`.

Post-Build CRC Calculation
--------------------------

The post-build CRC calculation is done using a Python script which is present in the ``tools``
directory of OpenBSW. This Python script reads the elf file which was built and calculates
the CRC over the memory regions which were specified in linker script and injects the calculated
CRC value back into the elf at ``__checksum_result`` symbol defined in the linker script.
This marker must be added to the linker script by the user, similar to the approach used in the
ReferenceApp. This post-build operation ensures that the elf file contains the necessary CRC
value for comparison during runtime.

Runtime CRC Validation
----------------------

The validation process involves comparing the runtime-calculated CRC value with the injected CRC
value. Any discrepancy between them may indicate memory corruption, which can trigger
romCheckMonitor to handle the error appropriately (e.g., by logging the error,
resetting the system) through the SafeSupervisor.

The final comparison of the pre-calculated and runtime calculated CRC looks like this, the value
from the linker script is compared with the output register of the CRC unit of the controller.

.. code-block:: cpp

    bool const crc_valid = (__checksum_result == crcRegisters.DATAu.DATA);
