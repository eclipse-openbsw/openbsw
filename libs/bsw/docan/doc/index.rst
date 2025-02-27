.. _docan:

docan - Diagnostics over CAN
============================

An implementation of the ISO-15765 specification.

``docan``'s purpose is to serve as a CAN Transport stack, allowing ECUs to send payloads of data
potentially too large to fit into a single CAN frame to each other via the CAN-TP protocol, which
will encode said payloads into CAN frames, splitting the payloads into several frames (or
reassembling messages from several frames) as needed.

If you're unfamiliar with the basics of the protocol, :ref:`can_tp_basics` is a good primer on the
essentials you need to know to understand CAN-TP and how it works.

Integrating projects must support and integrate the ``async`` module, as ``docan`` relies on
``async`` internally to execute several operations asynchronously. The ``async`` module also ensures
that all business logic operations run within the same task context, reducing the need for costly
interrupt-disabling locks for data access synchronization.

.. toctree::
   :maxdepth: 2

   user/index
