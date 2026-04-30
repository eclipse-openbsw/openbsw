ECC Check
=========

The **ECC (Error Correction Code)** Check feature is implemented to enhance data integrity by
detecting and correcting errors in memory, ensuring the robustness of data protection and
system reliability.

* **Single-bit Errors**:
  In the event of a single-bit error, whether in RAM or Flash, the ECC module automatically
  detects and corrects the error without requiring user intervention, maintaining seamless
  operation.

* **Double-bit Errors**:
  For double-bit errors, the system leverages the **ERM (Error Reporting Module)** to detect
  the error and initiate an interrupt to notify the system for further handling, ensuring
  critical errors are addressed in real-time.

Integration
-----------

ECC errors can occur during regular memory access, which may produce a **HardFault**, or
during **DMA** operations, where the corresponding registers are set without generating a
HardFault. To handle these scenarios, two approaches are implemented.

**Approach 1: ECC Error Detection During System Faults**
Usually, ECC triggers a HardFault, so in the HardFault handler it is checked if the HardFault is
caused by an ECC error. If confirmed, the system notifies the **SafeSupervisor** through
``dmaEccMonitor``, then logs the error, and finally initiates a system reset. In this case, an
**interrupt** is part of the standard fault handling mechanism.

**Approach 2: Periodic ECC Error Detection**
For all cases where a HardFault is not triggered—such as an ECC error during **DMA** transfers, the
relevant ECC error registers must be checked on a regular basis. If a double-bit error in RAM or
Flash is detected, the system notifies the **SafeSupervisor** through ``dmaEccMonitor``,
then logs the error, and then performs a system reset.

Cyclic ECC Check Period
-----------------------

To ensure consistent operation, the **cyclic ECC check** should be called at regular intervals.
The **SafeMemory cyclic** function triggers the ECC checks, which are invoked from the cyclic
function of the **SafetyManager**. This ensures that ECC checks are performed periodically, based
on the project's safety analysis.
