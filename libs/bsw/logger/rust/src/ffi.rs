// Copyright 2026 Accenture.

#![allow(static_mut_refs)]

unsafe extern "C" {
    pub fn bsw_cpp_logger_log(component_index: u8, level: Level, str: *const core::ffi::c_char);
}

#[unsafe(no_mangle)]
static mut LOGGER: crate::BswLogger = crate::BswLogger::create();

pub(crate) type ComponentIndex = u8;

/// Set the fallback logger component for messages routed through the `log` facade
/// (e.g. plain `log::info!(...)` calls from this crate or third-party deps).
///
/// `component_index` is the C++ `::util::logger::<NAME>` symbol (a `uint8_t`)
/// populated by the logger composition. Pass it directly rather than a name to
/// avoid an unnecessary lookup.
///
/// If this is not called, log messages routed through the `log` facade are silently
/// dropped. Rust code using the per-component `bsw_*!` macros is unaffected.
#[unsafe(no_mangle)]
pub extern "C" fn set_default_component(component_index: u8) {
    // SAFETY: called from C++ during single-threaded startup, before
    // `install_rust_logging` and any other task, so `LOGGER` is not aliased.
    unsafe { LOGGER.set_default_component(component_index) };
}

/// The rust logger must be installed after configuration
#[unsafe(no_mangle)]
pub extern "C" fn install_rust_logging() {
    // SAFETY: called once from C++ during single-threaded startup; `LOGGER` is a
    // `static`, satisfying the `&'static self` the `log` facade requires.
    unsafe { LOGGER.install_logger() };
}

#[allow(dead_code)]
#[allow(clippy::enum_variant_names)]
/// cbindgen:no-export
#[repr(C)]
#[derive(Debug)]
pub enum Level {
    LevelDebug = 0,
    LevelInfo = 1,
    LevelWarn = 2,
    LevelError = 3,
    LevelCritical = 4,
    LevelNone = 5,
}
