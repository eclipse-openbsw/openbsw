..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _logger_console:


Logger Console
==============

The console logger helps the user to switch between the logger level.

Overview
--------

Log level
---------

Log levels categorize the importance of log messages. It helps to filter and
understand the system's status and issues. Highest priorities log level will
disable all the least priorities log level messages.


**DEBUG** log level has least priorities and **CRITICAL** log level has highest priorities.

.. csv-table::
   :widths: 20,20

   "**Log level**", "**Priority**"
   "``DEBUG``", "0"
   "``INFO``", "1"
   "``WARN``", "2"
   "``ERROR``", "3"
   "``CRITICAL``", "4"
   "``NONE``", "5"

Console logger command
----------------------

The log level can be set using the command in console. The default log level is DEBUG.

      The console command has the following structure:

        ``logger level [<component>] [<new_level>]``

Examples
++++++++
  - ``logger level`` - Print the current logging level per component.
  - ``logger level INFO`` - Set logging level to INFO for all components.
  - ``logger level DEMO DEBUG`` - Set logging level to DEBUG for the DEMO component only.

Next: :ref:`digital_io`