.. _safeIo:

safeIo
======

``safeIo`` monitors the IO configuration registers and provides methods to enter and leave IO safe state.
If any unexpected change in the IO configuration is detected, it sends an event to the :ref:`safeSupervisor`.

.. tolerant-toctree::
    :maxdepth: 1

    user/index
