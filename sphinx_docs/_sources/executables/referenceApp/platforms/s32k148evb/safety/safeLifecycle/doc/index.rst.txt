.. _safeLifecycle:

safeLifecycle
=============

The ``safeLifecycle`` module serves as the central point for safety application and features. All
safety-related init, cyclic and shutdown operations are centralized within the SafetyManager,
making it easier to maintain a single safety object that can be connected to the lifecycle.
This simplifies code readability, reviews and supports easier optimization.

.. tolerant-toctree::

    user/index
