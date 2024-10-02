Startup
=======

Overview
--------

Below is the startup sequence.

* Initialization. The application initializes the FreeRTOS tasks (with one
  applicational task, idle task and timer task) and the calls created by a call
  to FreeRtosAdapter::init().

* Running the adapter. The application calls FreeRtosAdapter::run() to start
  the FreeRTOS scheduler.Before running FreeRTOS typically an initial Runnable will be triggered
  by the application. This will then be executed once FreeRTOS has started and the corresponding
  task is executed first (depending on its priority). If all tasks have been started and waiting
  for further events FreeRTOS triggers repeatedly the idle routine from within the idle task.

Code generation
---------------

Not applicable

Configuration
-------------

Not applicable

Calibration
-----------

Not applicable

Usage Examples and Integration
------------------------------

Should be synced together with a specific implementation ``asyncFreeRtos`` module.