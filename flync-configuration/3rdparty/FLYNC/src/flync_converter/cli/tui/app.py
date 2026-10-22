"""FlyncTUI Textual application and entry point."""

from __future__ import annotations

import logging
import threading
from typing import Optional

from textual.app import App, ComposeResult
from textual.containers import Horizontal
from textual.widgets import Button, Footer, Header, Log, Select, Static

from flync_converter import Converter
from flync_converter.base import ConverterConfig
from flync_converter.registry import registry

from .widgets import ConverterPanel, LogWidgetHandler

_CSS = """
Screen {
    background: $surface;
}

#panels {
    height: 1fr;
}

#run-log {
    height: 5;
    border: solid $accent;
    margin: 0 1;
}

#btn-row {
    height: auto;
    padding: 0 1;
    align: left middle;
}

#log-level-select {
    width: 16;
    margin: 0 1;
}

#btn-row-spacer {
    width: 1fr;
}

Button {
    margin: 0 1;
}
"""


class FlyncTUI(App):
    """flync Converter — interactive TUI (single-screen split-panel)."""

    TITLE = "flync Converter"
    SUB_TITLE = "Interactive format conversion"
    CSS = _CSS
    BINDINGS = [("ctrl+q", "quit", "Quit")]

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._log_handler: Optional[LogWidgetHandler] = None

    _LOG_LEVELS = [
        ("DEBUG", "DEBUG"),
        ("INFO", "INFO"),
        ("WARNING", "WARNING"),
        ("ERROR", "ERROR"),
    ]
    _DEFAULT_LOG_LEVEL = "WARNING"

    def on_mount(self) -> None:
        """Attach a log handler that routes Python log records to the on-screen Log widget."""
        handler = LogWidgetHandler(self.query_one("#run-log", Log))
        handler.setFormatter(logging.Formatter("%(levelname)s %(name)s: %(message)s"))
        handler.setLevel(self._DEFAULT_LOG_LEVEL)
        logging.getLogger().setLevel(self._DEFAULT_LOG_LEVEL)
        logging.getLogger().addHandler(handler)
        self._log_handler = handler

    def on_unmount(self) -> None:
        """Remove the log handler when the TUI app exits."""
        if self._log_handler is not None:
            logging.getLogger().removeHandler(self._log_handler)
            self._log_handler = None

    def compose(self) -> ComposeResult:  # NOSONAR
        """Build the TUI layout: source/dest converter panels, log output, and control buttons."""
        yield Header()
        with Horizontal(id="panels"):
            yield ConverterPanel("Source", default="flync", id="source-panel")
            yield ConverterPanel("Destination", default="flync", id="dest-panel")
        yield Log(highlight=True, id="run-log")
        with Horizontal(id="btn-row"):
            yield Select(
                self._LOG_LEVELS,
                id="log-level-select",
                allow_blank=False,
                value=self._DEFAULT_LOG_LEVEL,
            )
            yield Static("", id="btn-row-spacer")
            yield Button("Convert", id="btn-convert", variant="primary", disabled=True)
        yield Footer()

    # ------------------------------------------------------------------
    # Panel events
    # ------------------------------------------------------------------

    def on_converter_panel_changed(self, event: ConverterPanel.Changed) -> None:
        """Re-evaluate whether the Convert button should be enabled when a panel selection changes."""
        self._refresh_convert_button()

    def on_select_changed(self, event: Select.Changed) -> None:
        """Update the active log level when the log-level selector changes."""
        if event.select.id == "log-level-select" and event.value is not Select.BLANK:
            level = str(event.value)
            logging.getLogger().setLevel(level)
            if self._log_handler is not None:
                self._log_handler.setLevel(level)

    def _refresh_convert_button(self) -> None:
        source = self.query_one("#source-panel", ConverterPanel)
        dest = self.query_one("#dest-panel", ConverterPanel)
        self.query_one("#btn-convert", Button).disabled = not (source.converter_type and dest.converter_type)

    # ------------------------------------------------------------------
    # Conversion
    # ------------------------------------------------------------------

    def on_button_pressed(self, event: Button.Pressed) -> None:
        """Read configs from both panels and run the conversion when the Convert button is pressed."""
        if event.button.id != "btn-convert":
            return

        source = self.query_one("#source-panel", ConverterPanel)
        dest = self.query_one("#dest-panel", ConverterPanel)

        try:
            source_cfg = source.read_config()
            source.clear_error()
        except Exception as exc:
            source.show_error(str(exc))
            return

        try:
            dest_cfg = dest.read_config()
            dest.clear_error()
        except Exception as exc:
            dest.show_error(str(exc))
            return

        self._run_conversion(
            source.converter_type,  # type: ignore[arg-type]
            source_cfg,
            dest.converter_type,  # type: ignore[arg-type]
            dest_cfg,
        )

    def _run_conversion(
        self,
        source_type: str,
        source_cfg: ConverterConfig,
        dest_type: str,
        dest_cfg: ConverterConfig,
    ) -> None:
        log = self.query_one("#run-log", Log)
        btn = self.query_one("#btn-convert", Button)
        btn.disabled = True
        log.clear()
        log.write_line(f"Starting: {source_type} -> {dest_type}")

        def _worker() -> None:
            """Run the conversion in a background thread and report result to the log widget."""
            try:
                Converter().convert(
                    source=source_cfg.config_path,
                    destination=dest_cfg.config_path,
                    source_type=source_type,
                    destination_type=dest_type,
                    source_config=source_cfg,
                    destination_config=dest_cfg,
                )
                self.app.call_from_thread(log.write_line, "Conversion completed successfully.")
            except Exception as exc:
                self.app.call_from_thread(log.write_line, f"Conversion failed: {exc}")
            finally:
                self.app.call_from_thread(self._refresh_convert_button)

        threading.Thread(target=_worker, daemon=True).start()


def run_tui() -> None:
    """Load plugins and launch the Textual TUI."""
    registry.load_plugins()
    FlyncTUI().run()
