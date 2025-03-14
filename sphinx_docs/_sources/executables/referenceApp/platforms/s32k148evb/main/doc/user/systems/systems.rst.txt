.. _systems:

Systems
=======

Systems, bundle all instances of BSW classes needed for a particular use case and group those
instances into logical segments.

Systems defined under `platform/s32k1xx/main/systems` are responsible for managing the
hardware modules.

The following table provides details about the context and runlevels for each system:

.. toctree::
   :hidden:

   BspSystem
   CanSystem

.. csv-table::
   :widths: 20,50,20,10
   :width: 100%

   **System**, **Info**, **Context**, **Runlevel**
   `BspSystem`, "System to manage BSP modules", "TASK_BSP", "1"
   `CanSystem`, "System to manage CAN module", "TASK_CAN", "2"
