Functional Safety
=================

OpenBSW provides several mechanisms to support compliance with functional safety according to
ISO 26262. These mechanisms are demonstrated in the ReferenceApp.

The implementation includes the following features:

- Watchdog, including startup test to verify the correct operation of watchdog
- MPU configuration to protect the safety RAM and stack memory regions
- ECC for RAM and flash memory
- CRC calculations for specific ROM memory regions
- Monitoring IO configuration
- Event handling for safety related events. For instance, if the watchdog startup test fails
  (indicates the watchdog is not functioning properly), the watchdog startup check monitor will
  be triggered with a test failure event, this event will be handled by the safeSupervisor's event
  handling method, which logs the error message and causes the software system reset.

Supported Platforms
-------------------

The generic modules can be used on any platform, but functional safety always depends on platform
specific features and must be developed according to the safety manual of the controller, project
requirements and stakeholder needs.

The implementation of e.g. MPU, ECC or watchdog in ReferenceApp is specific to S32K1XX platform.

Code Partition
--------------

It is recommended to split the safety code into following sections as shown in the ReferenceApp:

.. list-table::
   :widths: auto
   :stub-columns: 0

   * - Generic section
     - The safety code which is not specific to a project or platform is placed in generic folder
       `libs/safety`.
   * - Platform specific section
     - The safety code which is specific to a platform is placed in
       `platforms/<platform_name>/safety`.
   * - Project specific but platform independent
     - The safety code which is specific to a project but not specific to any platform is placed
       in the project specific folder in the following path `executables/referenceApp/safety`.
   * - Project and platform specific section
     - The safety code which is specific to a project and also specific to a platform is placed in
       `executables/referenceApp/platforms/<platform_name>/safety`.

Safety Modules
--------------

The safety implementation consists of following modules:

.. list-table::
   :widths: auto
   :stub-columns: 0

   * - :ref:`safeMemory`
     - Implements the MPU, defines the access rights for safety RAM and stack memory regions using
       MPU. Provides ECC support for RAM and flash memory and includes a ROM check
       that calculates the CRC for specific ROM regions.
   * - :ref:`safeLifecycle`
     - This is the central point where all the initialization and function calls of the safety
       relevant components happen.
   * - :ref:`safeSupervisor`
     - Implements the handling of safety-related events.
   * - :ref:`safeWatchdog`
     - Manages the watchdog and provides the safety checks.
   * - :ref:`safeIo`
     - Monitors the configuration registers of safety relevant IOs and provides methods to
       enter and leave a safe state for IOs.
   * - :ref:`safeMonitor`
     - A collection of templated classes which formalize a condition, its check and the reporting
       event if the condition isn't met.
   * - :ref:`safeUtils`
     - Provides helper classes like the declaration for the logger component.

Watchdog Test
-------------

.. note::
    This test resets the system twice, so if you notice two watchdog resets after the control
    reaches the main function of S32K1XX platform, then that is because of watchdog fast test.

Before using the watchdog in application code it is required to verify that it is functioning
correctly, this verification is performed by the Watchdog Fast Test, which is implemented in
:ref:`safeBspMcuWatchdog` module for S32K148 platform and at the very beginning as soon as
possible to minimize the startup delay.

The `executeFastTest` method of `Watchdog` class executes the watchdog test. The following
pseudocode demonstrates how this method is used in the main function:

.. code-block:: C++

  // enters the limp home mode
  enterLimpHome();
  bool const testResult = executeFastTest(timeout_delay);
  if (testResult)
  {
     // test is successful, so logs the test success message and leaves the limp home mode
      leaveLimpHome();
  }
  else
  {
      // triggers the watchdog startup check monitor which causes the reset
      watchdogStartupCheckMonitor.trigger();
  }

Initialization and Usage
------------------------

ReferenceApp serves as a demonstration and the following things have to be adjusted accordingly by
the user of OpenBSW. The project specific parts must be added by the user.

SafetyManager
+++++++++++++

SafetyManager is the central point of safety application and the features. Centralize all
init/cyclic calls here. The SafetyManager is called from safety task.

Watchdog
++++++++

Initialize safeWatchdog as early as possible and service the watchdog at regular intervals, choose
the watchdog timeout value and service time according to the project requirements. For demonstration
purposes, in ReferenceApp, 250 milliseconds is used as the watchdog timeout value and 80
milliseconds is chosen as the watchdog service time. Depending on the project, the watchdog
is always active or can be disabled in certain situations like software update or shutdown.
In ReferenceApp, the watchdog is disabled during shutdown of the safety task.

Memory Protection Unit
++++++++++++++++++++++

The memory regions are project specific. In ReferenceApp, the initialized safety relevant data is
mapped to .mpu_data/.mpu_rodata section and uninitialized data relevant to safety is mapped
to .mpu_bss section in the linkerscript.
The linkerscript asserts that both .mpu_data and .mpu_rodata are empty. Since data should not be
initialized/assigned before mpu is enabled. All mpu-related global variables are placed in
.mpu_bss and explicitly initialized/assigned while mpu is enabled.

The memory is divided into several sections, which are configured by the MPU.
Among these sections, the protected RAM and safety task stack contain safety-relevant data. All
sections are always readable and writeable, but the protected RAM is writeable only from safety
context and safety task stack is not writeable when an ISR interrupts the safety context. As ISRs
can interrupt safety context, they are wrapped with the pre and post ISR hooks. These hooks revoke
write access to the protected RAM and safety stack regions before the execution of ISR and re-enable
write access to these regions after the ISR execution is complete. Every data written into those
sections before MPU is enabled, is not reliable. So, initialization of safety applications must be
done after MPU init.

IOs
+++

The values of inputs cannot be validated without context, e.g. depending on the system state,
only certain values are valid. Appropriate checks must be implemented in application specifics
software components.

On a generic level, the configuration of IOs can be monitored. The easiest way is to lock the
configuration registers at startup. Afterwards they must be checked once to ensure that
the right configuration is locked. Configuration registers, which cannot be locked, must be checked
cyclically.

ROM Check
+++++++++

In ReferenceApp, The ROM check feature performs a single runtime CRC calculation over the memory
regions which contain safety relevant data or code. The ROM check is initialized from SafetyManager
and is performed when the system is in an idle state, ensuring minimal impact on system performance
and avoiding any increase in startup time.

Future Additions
----------------

- In future, no-init RAM will be incorporated. No-init RAM is a region of memory that is not
  initialized during the startup process. Usually, RAM is zeroed out at the start except this
  section to retain data that remains available after a reset. So, currently error handling is done
  by simply logging an error message followed by software reset, later logging will be replaced with
  writing the event to no-init RAM. In future, a more sophisticated error handling will be added,
  like writing an error entry to non-volatile memory after the reset.

- MPU startup test will be added, after incorporating no-init RAM.
