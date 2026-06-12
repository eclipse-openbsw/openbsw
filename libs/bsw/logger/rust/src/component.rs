// Copyright 2026 Accenture.

//! Per-component logger handles backed by a C++-side getter function.
//!
//! Each [`LoggerComponent`] wraps a `bsw_logger_component_<NAME>()` extern that
//! returns the current `::util::logger::<NAME>` index. The C++ macros
//! `DECLARE_LOGGER_COMPONENT` / `DEFINE_LOGGER_COMPONENT` emit those getters
//! automatically. Use [`declare_logger_component!`](crate::declare_logger_component)
//! to bind one on the Rust side.

/// Sentinel value indicating an unresolved component (matches C++ `COMPONENT_NONE`).
pub const COMPONENT_NONE: u8 = 0xFF;

/// Handle for a C++ logger component, resolved on each call via an extern getter.
///
/// To prevent init ordering issues there is no caching. The value is whatever C++
/// currently holds in `::util::logger::<NAME>`.
pub struct LoggerComponent {
    resolver: unsafe extern "C" fn() -> u8,
}

// SAFETY: the only field is an immutable fn pointer; the getter just reads a
// `uint8_t` C++ sets once at init, so sharing across threads is sound.
unsafe impl Sync for LoggerComponent {}

impl LoggerComponent {
    /// Bind a component to its C++-side getter. Use 
    /// [`declare_logger_component!`](crate::declare_logger_component) instead
    /// of constructing this directly.
    pub const fn new(resolver: unsafe extern "C" fn() -> u8) -> Self {
        Self { resolver }
    }

    /// Fetch the current component index from C++.
    #[inline]
    pub fn index(&self) -> u8 {
        // SAFETY: `resolver` is the C++ `bsw_logger_component_<NAME>()` getter;
        // it takes no args, returns a `uint8_t`, and has no preconditions.
        unsafe { (self.resolver)() }
    }
}
