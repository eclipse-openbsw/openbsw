DummySessionPersistence
=======================

``DummySessionPersistence`` class is a derived class of ``ISessionPersistence``
used as a default implementation where session persistence is required but not
yet implemented or needed. This class overrides two methods ``readSession`` and
``writeSession``.

.. csv-table::
   :widths: 20,100

   "``readSession()``", "Reads session data and takes a `DiagnosticSessionControl` parameter."
   "``writeSession()``", "Writes session data and takes a `DiagnosticSessionControl` parameter."