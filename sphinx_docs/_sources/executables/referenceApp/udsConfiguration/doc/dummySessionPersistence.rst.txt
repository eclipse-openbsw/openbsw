..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

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