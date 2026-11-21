"""FlyncGUI — PySide6 GUI application for flync Converter."""

from __future__ import annotations

import logging
import sys
import threading
from typing import Optional

from PySide6.QtCore import Qt, Signal
from PySide6.QtWidgets import (
    QApplication,
    QComboBox,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QPlainTextEdit,
    QPushButton,
    QSplitter,
    QVBoxLayout,
    QWidget,
)

from flync_converter import Converter
from flync_converter.base import ConverterConfig
from flync_converter.registry import registry

from .widgets import ConverterPanel, LogWidgetHandler

_LOG_LEVELS = ["DEBUG", "INFO", "WARNING", "ERROR"]
_DEFAULT_LOG_LEVEL = "WARNING"


class FlyncGUI(QMainWindow):
    """flync Converter — PySide6 GUI (split-panel, mirrors the Textual TUI)."""

    # Signals for thread-safe log appending and button refresh from workers
    _log_signal = Signal(str)
    _done_signal = Signal()

    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("flync Converter")
        self.resize(960, 640)
        self._log_handler: Optional[LogWidgetHandler] = None
        self._setup_ui()
        self._setup_logging()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _setup_ui(self) -> None:
        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)
        root.setContentsMargins(8, 8, 8, 8)
        root.setSpacing(6)

        # Split panels (resizable)
        splitter = QSplitter(Qt.Orientation.Horizontal)
        self._source_panel = ConverterPanel("Source", default="flync")
        self._dest_panel = ConverterPanel("Destination", default="flync")
        self._source_panel.changed.connect(self._refresh_convert_button)
        self._dest_panel.changed.connect(self._refresh_convert_button)
        splitter.addWidget(self._source_panel)
        splitter.addWidget(self._dest_panel)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 1)
        root.addWidget(splitter, stretch=1)

        # Log output
        self._log_view = QPlainTextEdit()
        self._log_view.setReadOnly(True)
        self._log_view.setMaximumHeight(130)
        self._log_view.setPlaceholderText("Conversion log output will appear here…")
        root.addWidget(self._log_view)

        # Bottom bar: log level + convert button
        btn_row = QHBoxLayout()
        btn_row.addWidget(QLabel("Log level:"))
        self._log_level_combo = QComboBox()
        for level in _LOG_LEVELS:
            self._log_level_combo.addItem(level)
        self._log_level_combo.setCurrentText(_DEFAULT_LOG_LEVEL)
        self._log_level_combo.setFixedWidth(100)
        self._log_level_combo.currentTextChanged.connect(self._on_log_level_changed)
        btn_row.addWidget(self._log_level_combo)
        btn_row.addStretch()
        self._convert_btn = QPushButton("Convert")
        self._convert_btn.setEnabled(False)
        self._convert_btn.setFixedWidth(100)
        self._convert_btn.clicked.connect(self._on_convert_clicked)
        btn_row.addWidget(self._convert_btn)
        root.addLayout(btn_row)

        # Wire thread-safe signals
        self._log_signal.connect(self._log_view.appendPlainText)
        self._done_signal.connect(self._refresh_convert_button)

        self._refresh_convert_button()

    def _setup_logging(self) -> None:
        handler = LogWidgetHandler(self._log_view.appendPlainText)
        handler.setFormatter(logging.Formatter("%(levelname)s %(name)s: %(message)s"))
        handler.setLevel(_DEFAULT_LOG_LEVEL)
        logging.getLogger().setLevel(_DEFAULT_LOG_LEVEL)
        logging.getLogger().addHandler(handler)
        self._log_handler = handler

    def closeEvent(self, event) -> None:
        """Remove the log handler when the window is closed."""
        if self._log_handler is not None:
            logging.getLogger().removeHandler(self._log_handler)
            self._log_handler = None
        super().closeEvent(event)

    # ------------------------------------------------------------------
    # Event handlers
    # ------------------------------------------------------------------

    def _refresh_convert_button(self, *_) -> None:
        self._convert_btn.setEnabled(bool(self._source_panel.converter_type and self._dest_panel.converter_type))

    def _on_log_level_changed(self, level: str) -> None:
        logging.getLogger().setLevel(level)
        if self._log_handler:
            self._log_handler.setLevel(level)

    def _on_convert_clicked(self) -> None:
        try:
            source_cfg = self._source_panel.read_config()
            self._source_panel.clear_error()
        except Exception as exc:
            self._source_panel.show_error(str(exc))
            return

        try:
            dest_cfg = self._dest_panel.read_config()
            self._dest_panel.clear_error()
        except Exception as exc:
            self._dest_panel.show_error(str(exc))
            return

        self._run_conversion(
            self._source_panel.converter_type,  # type: ignore[arg-type]
            source_cfg,
            self._dest_panel.converter_type,  # type: ignore[arg-type]
            dest_cfg,
        )

    # ------------------------------------------------------------------
    # Conversion
    # ------------------------------------------------------------------

    def _run_conversion(
        self,
        source_type: str,
        source_cfg: ConverterConfig,
        dest_type: str,
        dest_cfg: ConverterConfig,
    ) -> None:
        self._convert_btn.setEnabled(False)
        self._log_view.clear()
        self._log_signal.emit(f"Starting: {source_type} -> {dest_type}")

        def _worker() -> None:
            """Run the conversion in a background thread and emit signals to update the UI."""
            try:
                Converter().convert(
                    source=source_cfg.config_path,
                    destination=dest_cfg.config_path,
                    source_type=source_type,
                    destination_type=dest_type,
                    source_config=source_cfg,
                    destination_config=dest_cfg,
                )
                self._log_signal.emit("Conversion completed successfully.")
            except Exception as exc:
                self._log_signal.emit(f"Conversion failed: {exc}")
            finally:
                self._done_signal.emit()

        threading.Thread(target=_worker, daemon=True).start()


def run_gui() -> None:
    """Load plugins and launch the PySide6 GUI."""
    registry.load_plugins()
    app = QApplication.instance() or QApplication(sys.argv)
    window = FlyncGUI()
    window.show()
    sys.exit(app.exec())
