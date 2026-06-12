// Copyright 2026 Accenture.

use crate::ffi;

pub struct BswLogger {
    pub(crate) default_component: Option<crate::ffi::ComponentIndex>,
}

// SAFETY: the only field is a plain `Option<u8>`, there are no pointers or interior
// mutability. So sending and sharing it across threads is sound.
unsafe impl Send for BswLogger {}
unsafe impl Sync for BswLogger {}

impl BswLogger {
    /// Create a new [BswLogger] without any forwarding configured
    pub const fn create() -> Self {
        Self {
            default_component: None,
        }
    }

    /// Set the fallback logger component used for any `log::*!` call that goes through
    /// the `log` facade (e.g. messages from third-party crates).
    ///
    /// The index must be the value of the `::util::logger::<NAME>` symbol assigned by the
    /// C++ logger composition (i.e. after `logger::init()` has run).
    ///
    /// When unset, such messages are silently dropped. Rust code that uses the
    /// per-component `bsw_*!` macros bypasses this fallback entirely.
    pub fn set_default_component(&mut self, component_index: crate::ffi::ComponentIndex) {
        self.default_component = Some(component_index);
    }

    /// Install the [BswLogger] as the system wide logger
    pub fn install_logger(&'static self) {
        log::set_logger(self).expect("failed to set logger");
        log::set_max_level(log::LevelFilter::Trace);
    }
}

impl log::Log for BswLogger {
    fn enabled(&self, _metadata: &log::Metadata) -> bool {
        // checked in cpp
        true
    }

    fn log(&self, record: &log::Record) {
        use core::fmt::Write;
        let Some(component_index) = self.default_component else {
            return;
        };
        let mut buf = crate::log_buf::LogBuf::<400>::default();
        // Prepend the target so consumers can tell which crate/module produced the
        // message. The default component is shared across all `log::*!` callers.
        let _ = write!(buf, "[{}] {}", record.target(), record.args());
        let level = {
            use ffi::Level::*;
            use log::Level::*;
            match record.level() {
                Error => LevelError,
                Warn => LevelWarn,
                Info => LevelInfo,
                Debug => LevelDebug,
                // The C++ logger has no Trace level. Map to the nearest real level instead.
                Trace => LevelDebug,
            }
        };
        // SAFETY: `component_index` is the configured default; `buf` is a
        // NUL-terminated buffer that outlives this synchronous call.
        unsafe { crate::ffi::bsw_cpp_logger_log(component_index, level, buf.as_raw_pointer()) };
    }

    fn flush(&self) {}
}

#[cfg(test)]
mod tests {
    extern crate std;

    use super::*;
    use crate::ffi::Level;
    use core::ffi::CStr;
    use log::Log;
    use std::string::{String, ToString};
    use std::sync::{Mutex, MutexGuard};
    use std::vec::Vec;

    #[derive(Debug, PartialEq, Eq)]
    struct LogCall {
        component_index: u8,
        level: u8,
        message: String,
    }

    static CALLS: Mutex<Vec<LogCall>> = Mutex::new(Vec::new());
    static TEST_LOCK: Mutex<()> = Mutex::new(());

    /// Test stub for the C++-provided FFI symbol.
    #[unsafe(no_mangle)]
    extern "C" fn bsw_cpp_logger_log(
        component_index: u8,
        level: Level,
        s: *const core::ffi::c_char,
    ) {
        // SAFETY: the logger passes a pointer to a NUL-terminated buffer that is
        // alive for the duration of this call.
        let msg = unsafe { CStr::from_ptr(s) }
            .to_str()
            .expect("test messages must be utf-8")
            .to_string();
        CALLS.lock().unwrap().push(LogCall {
            component_index,
            level: level as u8,
            message: msg,
        });
    }

    /// Serialize tests that share `CALLS`; reset captures on entry.
    fn enter_test() -> MutexGuard<'static, ()> {
        let guard = TEST_LOCK
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        CALLS.lock().unwrap().clear();
        guard
    }

    fn take_calls() -> Vec<LogCall> {
        core::mem::take(&mut *CALLS.lock().unwrap())
    }

    #[test]
    fn log_routes_to_default_component_with_target_prefix() {
        let _g = enter_test();
        let mut logger = BswLogger::create();
        logger.set_default_component(11);

        logger.log(
            &log::Record::builder()
                .target("unknown::mod")
                .level(log::Level::Error)
                .args(format_args!("boom"))
                .build(),
        );

        let calls = take_calls();
        assert_eq!(calls.len(), 1);
        assert_eq!(calls[0].component_index, 11);
        assert_eq!(calls[0].level, Level::LevelError as u8);
        assert_eq!(calls[0].message, "[unknown::mod] boom");
    }

    #[test]
    fn log_drops_message_when_no_default() {
        let _g = enter_test();
        let logger = BswLogger::create();

        logger.log(
            &log::Record::builder()
                .target("nope")
                .level(log::Level::Info)
                .args(format_args!("ignored"))
                .build(),
        );

        assert!(take_calls().is_empty());
    }

    #[test]
    fn log_maps_all_log_levels_to_ffi_levels() {
        let _g = enter_test();
        let mut logger = BswLogger::create();
        logger.set_default_component(0);

        for (in_level, expected) in [
            (log::Level::Error, Level::LevelError as u8),
            (log::Level::Warn, Level::LevelWarn as u8),
            (log::Level::Info, Level::LevelInfo as u8),
            (log::Level::Debug, Level::LevelDebug as u8),
            (log::Level::Trace, Level::LevelDebug as u8),
        ] {
            logger.log(
                &log::Record::builder()
                    .target("k")
                    .level(in_level)
                    .args(format_args!("x"))
                    .build(),
            );
            let calls = take_calls();
            assert_eq!(calls.len(), 1, "{in_level:?}");
            assert_eq!(calls[0].level, expected, "{in_level:?}");
        }
    }
}
