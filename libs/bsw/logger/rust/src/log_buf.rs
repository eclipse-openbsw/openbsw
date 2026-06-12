// Copyright 2026 Accenture.

pub struct LogBuf<const LOG_BUF_SIZE: usize> {
    buffer: [u8; LOG_BUF_SIZE],
    bytes_used: usize,
}

impl<const LOG_BUF_SIZE: usize> LogBuf<LOG_BUF_SIZE> {
    pub fn as_raw_pointer(&self) -> *const core::ffi::c_char {
        &self.buffer as *const u8 as *const core::ffi::c_char
    }
}

impl<const LOG_BUF_SIZE: usize> Default for LogBuf<LOG_BUF_SIZE> {
    fn default() -> Self {
        Self {
            buffer: [0; LOG_BUF_SIZE],
            bytes_used: 0,
        }
    }
}

impl<const LOG_BUF_SIZE: usize> core::fmt::Write for LogBuf<LOG_BUF_SIZE> {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        let s = s.as_bytes();
        if self.bytes_used + s.len() < LOG_BUF_SIZE {
            // at least one null byte remaining
            for &b in s {
                self.buffer[self.bytes_used] = b;
                self.bytes_used += 1;
            }
            Ok(())
        } else {
            // not enough space
            // only copy what fits
            let s = &s[0..(LOG_BUF_SIZE - self.bytes_used - 1)];
            for &b in s {
                self.buffer[self.bytes_used] = b;
                self.bytes_used += 1;
            }
            Err(core::fmt::Error)
        }
    }
}

#[cfg(test)]
mod test {
    use crate::log_buf::LogBuf;
    use core::fmt::Write;

    #[test]
    fn test_format_into_logbuf_until_full() {
        const BUF_SIZE: usize = 20;
        let mut buf = LogBuf::<BUF_SIZE>::default();
        assert!(write!(buf, "").is_ok());
        assert_eq!(buf.bytes_used, 0);
        assert!(buf.buffer.starts_with(b"\0"));

        assert!(write!(buf, "Hello World!").is_ok());
        assert_eq!(buf.bytes_used, "Hello World!".len());
        assert!(buf.buffer.starts_with(b"Hello World!\0"));

        assert!(write!(buf, "Hello World!").is_err());
        assert_eq!(buf.bytes_used, BUF_SIZE - 1);
        assert!(buf.buffer.starts_with(b"Hello World!Hello W\0"));
    }

    #[test]
    fn test_format_into_logbuf_direct_full() {
        const BUF_SIZE: usize = 20;
        let mut buf = LogBuf::<BUF_SIZE>::default();
        assert!(write!(buf, "Hello World!Hello World!").is_err());
        assert_eq!(buf.bytes_used, BUF_SIZE - 1);
        assert!(buf.buffer.starts_with(b"Hello World!Hello W\0"));
    }

    #[test]
    fn test_format_into_logbuf_one_byte_to_large() {
        const BUF_SIZE: usize = 20;
        let mut buf = LogBuf::<BUF_SIZE>::default();
        assert!(write!(buf, "Hello World!Hello Wo").is_err());
        assert_eq!(buf.bytes_used, BUF_SIZE - 1);
        assert!(buf.buffer.starts_with(b"Hello World!Hello W\0"));
    }

    #[test]
    fn test_format_into_logbuf_just_fits() {
        const BUF_SIZE: usize = 20;
        let mut buf = LogBuf::<BUF_SIZE>::default();
        assert!(write!(buf, "Hello World!Hello W").is_ok());
        assert_eq!(buf.bytes_used, BUF_SIZE - 1);
        assert!(buf.buffer.starts_with(b"Hello World!Hello W\0"));
    }
}
