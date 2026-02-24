.. _CanSystem:

CAN System
==========

Overview
--------
    The ``CanSystem`` implements the interface ``ICanSystem``. Additionally, it includes C-style
    functions for handling the CAN receive and transmit interrupt service routines (ISRs).

Constructor
+++++++++++

.. literalinclude:: ../../../include/systems/CanSystem.h
   :start-after: CONSTRUCTOR_START
   :end-before: CONSTRUCTOR_END

Public API
++++++++++

``CanSystem`` provides the following functions:

.. literalinclude:: ../../../include/systems/CanSystem.h
   :start-after: PUBLIC_API_START
   :end-before: PUBLIC_API_END
   :language: c++
   :dedent: 4

CanRxRunnable
-------------

Constructor
+++++++++++

.. literalinclude:: ../../../include/systems/CanSystem.h
   :start-after: CONSTRUCTOR1_START
   :end-before: CONSTRUCTOR1_END

Public API
++++++++++

``CanRxRunnable`` provides the following functions:

.. literalinclude:: ../../../include/systems/CanSystem.h
   :start-after: PUBLIC1_API_START
   :end-before: PUBLIC1_API_END
   :language: c++
   :dedent: 4

C - Style Functions
-------------------

.. literalinclude:: ../../../include/systems/CanSystem.h
   :start-after: EXTERN_START
   :end-before: EXTERN_END
   :language: c
