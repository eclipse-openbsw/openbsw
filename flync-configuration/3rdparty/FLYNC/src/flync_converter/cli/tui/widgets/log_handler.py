"""LogWidgetHandler — routes Python log records to a Textual Log widget."""

from __future__ import annotations

import logging

from textual.widgets import Log


class LogWidgetHandler(logging.Handler):
    """Forwards Python log records to a Textual Log widget."""

    def __init__(self, log_widget: Log) -> None:
        """Initialize the TUI log handler.

        Args:
            log_widget: Textual Log widget to write messages to.
        """
        super().__init__()
        self._log_widget = log_widget

    def emit(self, record: logging.LogRecord) -> None:
        """Format the log record and write it to the Textual Log widget, thread-safely."""
        try:
            line = self.format(record)
            app = self._log_widget.app
            try:
                # works when called from a background thread
                app.call_from_thread(self._log_widget.write_line, line)
            except RuntimeError:
                # called from the app's own event-loop thread
                app.call_later(self._log_widget.write_line, line)
        except Exception:
            self.handleError(record)
