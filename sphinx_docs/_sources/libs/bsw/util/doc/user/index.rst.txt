..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

User Documentation
==================

Overview
--------

The **util** module is a collection of various utilities and ready-to-use
solutions that may be directly integrated in the client code as **util**
library, including :ref:`commands <util_command>` for terminal,
:ref:`logging commands <util_logger>`, :ref:`memory manipulation
<util_memory>`, user managed memory, :ref:`streaming utilities
<util_stream>`, and :ref:`string wrapper <util_string>`. The **util**
module is intensively used throughout the project by other modules or
application code and is subject to update and expansion.

Submodules
----------

-  :ref:`util::command <util_command>` - command framework
-  :ref:`util::format <util_format>` - `printf`-like string formatting
-  :ref:`util::defere <util_defer>` - deferred function execution
-  :ref:`util::logger <util_logger>` - logging functionality
-  :ref:`util::memory <util_memory>` - memory management
-  :ref:`util::meta <util_meta>` - miscellaneous
-  :ref:`util::stream <util_stream>` - data i/o
-  :ref:`util::string <util_string>` - string handling

.. toctree::
   :hidden:

   command <command>
   format <format>
   defer <defer>
   logger <logger>
   memory <memory>
   meta <meta>
   stream <stream>
   string <string>
