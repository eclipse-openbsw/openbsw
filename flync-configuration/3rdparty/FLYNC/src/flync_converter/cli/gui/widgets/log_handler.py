"""LogWidgetHandler — routes Python log records to a QPlainTextEdit widget."""

from __future__ import annotations

import logging

from PySide6.QtCore import QObject, Signal


class _Signaller(QObject):
    """Qt object that exposes a thread-safe signal for log message delivery."""

    message = Signal(str)


class LogWidgetHandler(logging.Handler):
    """Forwards Python log records to a QPlainTextEdit via a Qt signal.

    Qt signals are thread-safe across thread boundaries, so this handler
    works correctly when called from background conversion threads.
    """

    def __init__(self, append_fn) -> None:
        """Initialize the log handler.

        Args:
            append_fn: Callable to invoke with formatted log messages.
        """
        super().__init__()
        self._signaller = _Signaller()
        self._signaller.message.connect(append_fn)

    def emit(self, record: logging.LogRecord) -> None:
        """Format the log record and emit it via the Qt signal."""
        try:
            self._signaller.message.emit(self.format(record))
        except Exception:
            self.handleError(record)
