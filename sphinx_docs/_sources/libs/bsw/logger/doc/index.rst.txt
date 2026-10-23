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
    literal ``"%s"`` format. We do **not** pass it as the format string directly
    because any ``%`` in the message would be read as a conversion specifier against
    an empty ``va_list`` (undefined behaviour / crash).

Level gating from Rust
    Both the ``bsw_*!`` macros and the ``log`` facade ask C++ whether the level is
    enabled (``bsw_cpp_logger_is_enabled``, mirroring
    ``::util::logger::Logger::isEnabled``) *before* formatting the message, and skip
    all formatting work when it is disabled. Without this, a disabled
    formatting-heavy log would still pay its full ``Display``/``Debug`` cost only to
    be dropped on the C++ side. The trade-off is one extra FFI call per enabled log
    site; The C++ ``Logger::log`` still re-checks the level, so the Rust check is an
    optimization, not the authority.

.. note::

    A log emitted from Rust is buffered twice before it reaches an output. The Rust
    side first formats the message into a stack ``LogBuf`` and passes the finished
    string across the FFI; ``bsw_cpp_logger_log`` then re-enters the normal C++ path,
    where ``BufferedLoggerOutput`` re-serializes the entry into its own ring buffer
    for later draining. Native C++ logs only incur the second of these steps. This is
    the cost of routing pre-formatted Rust strings through the shared C++ logger; the
    level gate above keeps it from being paid for entries that would be dropped.
