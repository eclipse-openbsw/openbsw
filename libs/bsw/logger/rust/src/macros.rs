// Copyright 2026 Accenture.

//! Macros for declaring and using logger components from Rust.
//!
//! 1. `declare_logger_component!(NAME);` at file scope binds a
//!    `pub static NAME: LoggerComponent` to the C++-side getter
//!    `bsw_logger_component_NAME()` emitted by `DECLARE_LOGGER_COMPONENT(NAME)`.
//! 2. `bsw_info!(NAME, "fmt {}", arg)` (and the other level macros) read the
//!    index via that getter and forward to the C++ logger.

/// Bind a Rust `LoggerComponent` to a C++ `DECLARE_LOGGER_COMPONENT(NAME)` symbol.
///
/// Emits an `extern "C"` declaration for `bsw_logger_component_<NAME>()` and a
/// `pub static <NAME>: LoggerComponent` that calls it on each use.
///
/// # Example
/// ```
/// use openbsw_logger::{declare_logger_component, bsw_info};
///
/// declare_logger_component!(DEMO);
///
/// fn greet() {
///     bsw_info!(DEMO, "Hello from Rust!");
/// }
/// ```
#[macro_export]
macro_rules! declare_logger_component {
    ($name:ident) => {
        $crate::__paste::paste! {
            unsafe extern "C" {
                fn [<bsw_logger_component_ $name>]() -> u8;
            }
            #[allow(non_upper_case_globals)]
            pub static $name: $crate::LoggerComponent =
                $crate::LoggerComponent::new([<bsw_logger_component_ $name>]);
        }
    };
}

/// Log a message at the given level to a [`LoggerComponent`](crate::LoggerComponent).
///
/// Bypasses the `log` crate; component lookup is a single FFI call.
/// The message is formatted into a stack buffer and forwarded to the C++ logger.
/// Prefer the level-specific macros ([`bsw_debug!`](crate::bsw_debug),
/// [`bsw_info!`](crate::bsw_info), etc.).
#[macro_export]
macro_rules! bsw_log {
    ($component:expr, $level_ffi:expr, $($arg:tt)+) => {{
        let comp = $component.index();
        if comp != $crate::component::COMPONENT_NONE {
            use core::fmt::Write;
            let mut buf = $crate::log_buf::LogBuf::<400>::default();
            let _ = write!(buf, $($arg)+);
            // SAFETY: `comp` was just checked to be a valid index, and `buf` is a
            // NUL-terminated buffer that outlives this synchronous call.
            unsafe {
                $crate::ffi::bsw_cpp_logger_log(comp, $level_ffi, buf.as_raw_pointer());
            }
        }
    }};
}

/// Log a message at DEBUG level. See [`declare_logger_component!`] for usage.
#[macro_export]
macro_rules! bsw_debug {
    ($component:expr, $($arg:tt)+) => {
        $crate::bsw_log!($component, $crate::ffi::Level::LevelDebug, $($arg)+)
    };
}

/// Log a message at INFO level.
#[macro_export]
macro_rules! bsw_info {
    ($component:expr, $($arg:tt)+) => {
        $crate::bsw_log!($component, $crate::ffi::Level::LevelInfo, $($arg)+)
    };
}

/// Log a message at WARN level.
#[macro_export]
macro_rules! bsw_warn {
    ($component:expr, $($arg:tt)+) => {
        $crate::bsw_log!($component, $crate::ffi::Level::LevelWarn, $($arg)+)
    };
}

/// Log a message at ERROR level.
#[macro_export]
macro_rules! bsw_error {
    ($component:expr, $($arg:tt)+) => {
        $crate::bsw_log!($component, $crate::ffi::Level::LevelError, $($arg)+)
    };
}

/// Log a message at CRITICAL level.
#[macro_export]
macro_rules! bsw_critical {
    ($component:expr, $($arg:tt)+) => {
        $crate::bsw_log!($component, $crate::ffi::Level::LevelCritical, $($arg)+)
    };
}
