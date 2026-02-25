// Copyright 2026 Accenture.

#![no_std]
//! Installs `softwareSystemReset` as the panic handler on no_std targets.
//! On std targets the std panic handler is used.
//!
//! This crate should be linked exactly once, typically in the top-level application crate.
//!
//! # Usage
//! To install this panic handler just declare it in your crate:
//! ```
//! extern crate openbsw_panic_handler;
//! ```

#[cfg(not(target_os = "none"))]
extern crate std;

#[cfg(target_os = "none")]
use core::panic::PanicInfo;

#[cfg(target_os = "none")]
unsafe extern "C" {
    safe fn softwareSystemReset() -> !;
}

/// Calls softwareSystemReset from bspMCU
#[cfg(target_os = "none")]
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    softwareSystemReset()
}
