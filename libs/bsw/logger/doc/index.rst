..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _logger:

logger - Logger module
======================

Overview
--------

The module `logger` provides an implementation of the interfaces declared in
:ref:`util_logger`.

Rust API
--------

The ``openbsw-logger`` crate (``libs/bsw/logger/rust``) lets Rust code log
through this C++ logger. See the crate's ``cargo doc`` for the full API; the
notes below cover the parts that are not obvious from the signatures.

Per-component logging (preferred for first-party Rust code)::

    use openbsw_logger::{declare_logger_component, bsw_info};

    declare_logger_component!(DEMO);   // binds to C++ ::util::logger::DEMO

    bsw_info!(DEMO, "value = {}", 42);

``declare_logger_component!(NAME)`` binds a ``LoggerComponent`` to the
``extern "C"`` getter that ``DECLARE_LOGGER_COMPONENT(NAME)`` emits on the C++
side. The component index is fetched from C++ on *every* log call rather than
cached. This is deliberate: the ``::util::logger::<NAME>`` symbols hold
``COMPONENT_NONE`` until ``logger::init()`` runs ``applyMapping()``, so caching
an index at Rust static-init time would capture ``COMPONENT_NONE`` and never
see the real value. Fetching live removes any init-ordering constraint.

``log`` facade fallback
    Plain ``log::info!`` calls (e.g. from third-party crates) are routed to a
    single default component configured from C++ via ``set_default_component``;
    if it is unset the message is dropped. The crate name is prepended to the
    message so the originating module is still visible.

.. warning::

    ``bsw_cpp_logger_log`` forwards the already-formatted Rust string via a
    literal ``"%s"`` format. Do **not** pass it as the format string directly:
    any ``%`` in the message would be read as a conversion specifier against an
    empty ``va_list`` (undefined behaviour / crash).