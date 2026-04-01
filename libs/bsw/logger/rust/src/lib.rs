// Copyright 2026 Accenture.

#![no_std]
//! Rust bridge to the OpenBSW C++ logger.
//!
//! # Per-component logging (preferred)
//!
//! For first-party Rust code, declare a logger component once per source file
//! and use the `bsw_*!` macros. The component index is fetched from C++ on each
//! call via an extern getter emitted by the C++ `DECLARE_LOGGER_COMPONENT` macro,
//! so no Rust-side init step is needed.
//!
//! ```
//! use openbsw_logger::{declare_logger_component, bsw_info, bsw_warn};
//!
//! declare_logger_component!(DEMO);
//!
//! fn run() {
//!     bsw_info!(DEMO, "value = {}", 42);
//!     bsw_warn!(DEMO, "something looks off");
//! }
//! ```
//!
//! # `log` facade fallback
//!
//! Plain `log::info!(...)` calls (e.g. from third-party crates) are routed to
//! a single default component, configured from C++ via
//! [`ffi::set_default_component`]. If unset, those messages are dropped.
//! Install the logger from C++ via [`ffi::install_rust_logging`] after the
//! C++ `logger::init()` has populated the component indices.

mod bsw_logger;
pub mod component;
pub mod ffi;
pub mod log_buf;
mod macros;

pub use bsw_logger::BswLogger;
pub use component::LoggerComponent;

#[doc(hidden)]
pub use paste as __paste;
