// Copyright 2026 Accenture.

//! A simple "Hello World" Rust library demonstrating Rust integration with OpenBSW.
//!
//! This crate provides a minimal example of how to expose Rust functions to C++ code
//! in the OpenBSW build system.
//!
//! # FFI Approach
//!
//! This example uses the **manual FFI approach**, where:
//! - Rust functions are marked with `#[no_mangle]` and `extern "C"`
//! - A corresponding C/C++ header file is written by hand
//!
//! This is not the only way to expose Rust to C++. Alternative approaches include:
//! - **[cbindgen](https://github.com/mozilla/cbindgen)**: Automatically generates C/C++ headers from Rust code
//! - **[cxx](https://cxx.rs/)**: Provides safe bidirectional interop between Rust and C++
//!
//! The manual approach was chosen here for simplicity.
//!
//! # Target Compatibility
//!
//! Uses `no_std` for compatibility with both POSIX and embedded targets.

#![no_std]

extern crate openbsw_console_out;
extern crate openbsw_panic_handler;

use core::fmt::Write;
use openbsw_console_out::Console;

/// Prints "Hello from Rust!" to BSP stdout via putByteToStdout.
#[unsafe(no_mangle)]
pub extern "C" fn rust_hello_world() {
    write!(Console, "Hello from Rust!\r\n").ok();
}

/// Adds two numbers and returns the result.
///
/// # Arguments
///
/// * `a` - First number to add
/// * `b` - Second number to add
///
/// # Returns
///
/// The sum of `a` and `b`.
#[unsafe(no_mangle)]
pub extern "C" fn rust_add(a: u32, b: u32) -> u32 {
    a.wrapping_add(b)
}
