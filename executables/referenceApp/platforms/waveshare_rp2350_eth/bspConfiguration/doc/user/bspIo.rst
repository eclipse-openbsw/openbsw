..
   *******************************************************************************
   Copyright (c) 2026 Roland Reichwein

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspConfig_Io_wsrp2350eth:

bspIo
=====

Configuration
-------------

``ioConfiguration.h`` is used to configure the input/output pins.

The `Io::PinConfiguration` structure in the ``ioConfiguration.h`` file is the configuration of
input/output (I/O) pins. Each member of the structure represents port, pin and direction of
specific pins.

The pin ID is used to refer to a specific pin in application code. The list of pin IDs present in
the ``PinId`` enum are corresponding to the array of `PinConfiguration`.



IO Modules
----------

.. toctree::
   :hidden:

   bspIo_input
   bspIo_output
   bspIo_outputPwm

.. csv-table::
   :widths: 30,70
   :width: 100%

   :ref:`bspIo_Input_wsrp2350eth`, "Input Configurations"
   :ref:`bspIo_Output_wsrp2350eth`, "Output Configurations"
   :ref:`bspIo_OutputPwm_wsrp2350eth`, "Output Pwm Configurations"