.. _docan:

docan - Diagnostics over CAN
============================

An implementation of the ISO-15765 specification.

``docan``'s purpose is to serve as a CAN Transport stack, allowing ECUs to send larger Transport
messages to each other via the CAN-TP protocol.

Prerequisites
-------------

Integrating projects must support and integrate the ``async`` module, as ``docan``
utilizes ``async`` internally for the execution of several operations asynchronously. ``async`` is
also used to ensure all business logic operations run in the same task context, minimizing or
entirely eliminating the need to use expensive interrupt-disabling locks for data access
synchronization.

.. toctree::
   :maxdepth: 1

   architecture

