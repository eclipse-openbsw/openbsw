.. _safeSupervisor:

safeSupervisor
==============

The ``SafeSupervisor`` implements the handling of safety-related events.
The SafeSupervisor class aggregates monitors responsible for detecting safety-critical conditions,
for example a watchdog sequence monitor or a MPU startup monitor. These monitors should be triggered
by corresponding system features.
If any monitor detects an unsafe condition (i.e., its condition is not met), it invokes the
SafeSupervisor's handle() function.

For more details on safety monitors, see the :ref:`safeMonitor` module.

In the reference application, the SafeSupervisor enters a "limp home" state in response to any
safety event. In other applications, the system's reaction may vary depending on the specific safety
concept and requirements.
