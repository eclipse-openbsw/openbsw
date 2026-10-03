..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

asyncThreadX
============

Introduction
------------

``asyncThreadX`` implements the :ref:`async` interface on top of **ThreadX**.

Memory Protection
-----------------

Background
++++++++++

A platform may configure the MPU so that each thread can access only its own stack.
Access to the stack of another thread causes an MPU fault, unless it is explicitly allowed.

Problem
+++++++

A variable passed to a ThreadX IPC API can be accessed by another thread.
Example: while a thread waits in ``tx_event_flags_get()``,
ThreadX keeps a pointer to the output variable.
A ``tx_event_flags_set()`` call from another thread writes through this pointer.
If the variable is on the stack, this write causes an MPU fault.

Design Choice
+++++++++++++

Keep such variables out of the stack, e.g. in ``.data`` or ``.bss``.
Then the MPU needs no explicit allow rule.

For example, the output variable of ``tx_event_flags_get()`` is a member (``_eventFlagsResult``)
of ``TaskContext`` and ``FutureSupport``, not a local variable.
Objects of these classes must not be on a stack.
