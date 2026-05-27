..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

timer
=====

Overview
--------

The ``timer`` module manages and processes both cyclic and single shot timeouts. It provides
functionality to set these timeouts, check if a timer is active and cancel the enqueued timeouts.
The ``Timer`` class uses the instances of ``Timeout`` class, where each instance represents individual
timeout.

.. toctree::
   :maxdepth: 1

   user/index