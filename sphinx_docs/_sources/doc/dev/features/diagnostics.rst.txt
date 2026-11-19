..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _feature_diagnostics:

Diagnostics (DoCAN / UDS)
=========================

Overview
--------

The reference application provides DoCAN (ISO 15765-2) transport and UDS diagnostics on
platforms that enable ``PLATFORM_SUPPORT_TRANSPORT`` and ``PLATFORM_SUPPORT_UDS``. The
features described here are platform independent; each platform selects them through its
``executables/referenceApp/platforms/<platform_name>/Options.cmake``.

Tester addressing
-----------------

With ``PLATFORM_SUPPORT_OBD_UDS_ADDRESSING`` enabled, the DoCAN channel uses ISO 15765-4
OBD-style tester addressing (0x7E0 request / 0x7E8 response, logical address 0x0600), so
standard UDS tester tools work without a custom channel configuration. Platforms without
the option keep the default example addressing.

Programming session
-------------------

``PLATFORM_SUPPORT_PROGRAMMING_SESSION`` adds an application-level UDS programming
session which keeps the UDS dispatcher alive instead of handing over to a bootloader.
See ``executables/referenceApp/udsConfiguration/src/uds/session/DiagSession.cpp`` for
the details.

Demo services
-------------

``PLATFORM_SUPPORT_UDS_DEMO_SERVICES`` registers example UDS services in the reference
application: ECU reset, an in-memory DTC store with read/clear/control services,
security access, VIN write and demo routine control. They give a connected tester a
complete diagnostic surface out-of-the-box and serve as worked examples for
implementing application-level UDS jobs on top of the UDS stack. The implementations
are located in ``executables/referenceApp/application/src/uds``.
