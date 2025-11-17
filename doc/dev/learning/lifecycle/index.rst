.. _learning_lifecycle:

Managing the lifecycle of components
====================================

Previous: :ref:`learning_unit_tests`

Using the BSW module ``lifecycle``, it is easy to divide an application's functionality into components
to be brought up / down in a specific order at different "run levels".
See :ref:`lifecycle` for a generic description of ``lifecycle``,
whereas its specific usage in this project is discussed below.

Firstly, note that this project uses `FreeRTOS <https://www.freertos.org/>`_ ported to each supported platform.
The BSW module ``async`` defines a generic interface for executing code in "contexts"
and the BSW module ``asyncFreeRtos`` implements the ``async`` interface for ``FreeRTOS``,
mapping each ``async`` "context" to a ``FreeRTOS`` "task".

The BSW module ``lifecycle`` defines the abstract class ``LifecycleComponent`` to represent each functional area.
Each instance of ``LifecycleComponent`` can be added to a single instance of ``LifecycleManager`` at a "run level"
to control when it is brought up / down in the application's lifecycle.
``AsyncLifecycleComponent`` and ``SingleContextLifecycleComponent`` are implementations of ``LifecycleComponent``
which allows the user to specify the ``async`` context in which to run.

Thus we can centrally organise both the order in which to bring up / down components and the contexts in which they run.

The convention followed in most projects using BSW is to define classes named "XyzSystem" for each functional area.
Each of these "System" classes implement ``LifecycleComponent`` and can thus be added to an instance of ``LifecycleManager``.
Below are examples of such "Systems"...

.. list-table::
    :header-rows: 1
    :stub-columns: 1
    :widths: auto

    * - Class Name
      - Platforms
      - LifecycleComponent type
      - Code location
    * - BspSystem
      - S32K148
      - SingleContextLifecycleComponent
      - platforms/s32k1xx/main/include/systems/BspSystem.h
        platforms/s32k1xx/main/src/systems/BspSystem.cpp
    * - RuntimeSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/RuntimeSystem.h
        executables/referenceApp/application/src/systems/RuntimeSystem.cpp
    * - CanSystem
      - POSIX
      - AsyncLifecycleComponent
      - executables/referenceApp/platforms/posix/main/include/systems/CanSystem.h
        executables/referenceApp/platforms/posix/main/src/systems/CanSystem.cpp
    * - CanSystem
      - S32K148
      - SingleContextLifecycleComponent
      - executables/referenceApp/platforms/s32k1xx/main/include/systems/CanSystem.h
        executables/referenceApp/platforms/s32k1xx/main/src/systems/CanSystem.cpp
    * - TransportSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/TransportSystem.h
        executables/referenceApp/application/src/systems/TransportSystem.cpp
    * - DoCanSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/DoCanSystem.h
        executables/referenceApp/application/src/systems/DoCanSystem.cpp
    * - UdsSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/UdsSystem.h
        executables/referenceApp/application/src/systems/UdsSystem.cpp
    * - SysAdminSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/SysAdminSystem.h
        executables/referenceApp/application/src/systems/SysAdminSystem.cpp
    * - DemoSystem
      - All
      - AsyncLifecycleComponent
      - executables/referenceApp/application/include/systems/DemoSystem.h
        executables/referenceApp/application/src/systems/DemoSystem.cpp


The function ``::app::run()`` found in the file ``executables/referenceApp/application/src/app/app.cpp``
adds components to the ``lifecycleManager`` at each run level.
Note that by using the function ``platformLifecycleAdd()``
(which has a different implementation on each platform)
each supported platform can flexibly include or exclude "Systems".

For example, the lifecycle on the POSIX platform is set up as follows...

.. list-table::
    :header-rows: 1
    :stub-columns: 1
    :widths: auto

    * - Run Level
      - Component
      - Contexts
      - where it is added to lifecycleManager
    * - 1
      - RuntimeSystem
      - TASK_BACKGROUND
      - executables/referenceApp/application/src/app/app.cpp
    * - 2
      - CanSystem
      - TASK_CAN
      - executables/referenceApp/platforms/posix/main/src/main.cpp
    * - 3
      - None
      -
      -
    * - 4
      - TransportSystem
      - TASK_UDS
      - executables/referenceApp/application/src/app/app.cpp
    * - 5
      - DoCanSystem
      - TASK_CAN
      - executables/referenceApp/application/src/app/app.cpp
    * - 6
      - UdsSystem
      - TASK_UDS
      - executables/referenceApp/application/src/app/app.cpp
    * - 7
      - SysAdminSystem
      - TASK_SYSADMIN
      - executables/referenceApp/application/src/app/app.cpp
    * - 8
      - DemoSystem
      - TASK_DEMO
      - executables/referenceApp/application/src/app/app.cpp

Explore the console lifecycle command ``lc`` to set different levels interactively.
Take time to study these classes and in particular the dependencies between them
for a better appreciation of how applications are organised.

Next: :ref:`learning_logging`
