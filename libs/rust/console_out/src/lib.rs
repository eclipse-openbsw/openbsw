// Copyright 2026 Accenture.

#![no_std]

/// Stdout console
///
/// implements [`core::fmt::write`]
pub struct Console;

impl core::fmt::Write for Console {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        for b in s.bytes() {
            // SAFETY: The printing function can be called with any u8.
            unsafe {
                putByteToStdout(b);
            }
        }
        Ok(())
    }
}

// SAFETY: The function declaration below is in line with the header `BspStubs.h``.
unsafe extern "C" {
    fn putByteToStdout(byte: u8);
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::fmt::Write;
    extern crate std;

    static TEST_OUTPUT: std::sync::Mutex<std::vec::Vec<u8>> =
        std::sync::Mutex::new(std::vec::Vec::new());

    #[unsafe(no_mangle)]
    extern "C" fn putByteToStdout(byte: u8) {
        TEST_OUTPUT.lock().unwrap().push(byte);
    }

    #[test]
    fn console_writes_string() {
        TEST_OUTPUT.lock().unwrap().clear();
        write!(Console, "hello\r\n").unwrap();
        assert_eq!(*TEST_OUTPUT.lock().unwrap(), b"hello\r\n");
    }

    #[test]
    fn console_writeln_newline_behavior() {
        TEST_OUTPUT.lock().unwrap().clear();
        writeln!(Console, "hello with newline?").unwrap();
        assert_eq!(*TEST_OUTPUT.lock().unwrap(), b"hello with newline?\n");
    }
}
