"""ConverterPanel widget — format selector + dynamic config form."""

from __future__ import annotations

import enum
import typing
from typing import Optional, Union

from rich.markup import escape
from textual.app import ComposeResult
from textual.containers import ScrollableContainer, Vertical
from textual.message import Message
from textual.widgets import Input, Label, Select, Static

from flync_converter.base import ConverterConfig
from flync_converter.registry import registry

from ..utils import cast_value, get_config_model


def _resolve_field_type(annotation):
    """Unwrap Optional/Union and return the inner type."""
    origin = getattr(annotation, "__origin__", None)
    if origin is Union:
        args = [a for a in typing.get_args(annotation) if a is not type(None)]
        return args[0] if args else annotation
    return annotation


class ConverterPanel(Vertical):
    """One half of the split-panel: format selector + dynamic config form."""

    DEFAULT_CSS = """
    ConverterPanel {
        width: 1fr;
        border: solid $accent;
        margin: 0 1;
    }

    ConverterPanel .panel-title {
        padding: 1 2;
        color: $accent;
        text-style: bold;
    }

    ConverterPanel Select {
        margin: 0 2 1 2;
    }

    ConverterPanel #fields {
        height: 1fr;
    }

    ConverterPanel .field-label {
        padding: 1 2 0 2;
        color: $text-muted;
    }

    ConverterPanel Input {
        margin: 0 2 1 2;
    }

    ConverterPanel .error-msg {
        padding: 0 2;
        color: $error;
        height: auto;
    }
    """

    _ERROR_WIDGET_ID = "#error"

    class Changed(Message):
        """Posted when the selected converter type changes."""

        def __init__(self, panel: "ConverterPanel", converter_type: Optional[str]) -> None:
            super().__init__()
            self.panel = panel
            self.converter_type = converter_type

        @property
        def control(self) -> "ConverterPanel":
            """Return the ConverterPanel that posted this message."""
            return self.panel

    def __init__(self, title: str, default: Optional[str] = None, **kwargs) -> None:
        """Initialize the TUI converter panel.

        Args:
            title: Title label for the panel (e.g., "Source" or "Destination").
            default: Optional default converter to select.
            **kwargs: Additional widget arguments.
        """
        super().__init__(**kwargs)
        self._title = title
        self._default = default
        self._converter_type: Optional[str] = None

    @property
    def converter_type(self) -> Optional[str]:
        """Return the currently selected converter type name, or None if none is selected."""
        return self._converter_type

    def compose(self) -> ComposeResult:  # NOSONAR
        """Build the panel: a title label, converter selector, dynamic config fields, and error display."""
        options = [(name, name) for name in registry.keys()]
        yield Label(self._title, classes="panel-title")
        yield Select(
            options,
            prompt=f"Select {self._title.lower()}…",
            id="select",
            value=(self._default if self._default in dict(options) else Select.BLANK),
        )
        yield ScrollableContainer(id="fields")
        yield Static("", id="error", classes="error-msg")

    async def on_select_changed(self, event: Select.Changed) -> None:
        """Rebuild config field widgets and post a Changed message when the converter selection changes."""
        if event.select.id != "select":
            return
        value = event.value
        self._converter_type = None if value is Select.BLANK else str(value)
        self.query_one(self._ERROR_WIDGET_ID, Static).update("")
        container = self.query_one("#fields", ScrollableContainer)
        await container.remove_children()
        if self._converter_type is not None:
            await container.mount(*self._make_field_widgets(self._converter_type))
        self.post_message(ConverterPanel.Changed(self, self._converter_type))

    def _make_field_widgets(self, converter_type: str) -> list:  # NOSONAR
        model = get_config_model(converter_type)
        widgets: list = []
        for name, fld in model.model_fields.items():
            required = fld.is_required()
            default = None if required else fld.get_default()
            field_type = _resolve_field_type(fld.annotation)
            prefix = "[red]*[/red] " if required else ""
            label_text = f"{prefix}{name}"
            if not required and default is not None:
                label_text += f" [dim](default: {default})[/dim]"
            widgets.append(Label(label_text, classes="field-label", markup=True))
            if isinstance(field_type, type) and issubclass(field_type, enum.Enum):
                options = [(m.name, m.value) for m in field_type]
                if isinstance(default, enum.Enum):
                    select_value = default.value
                elif required or not options:
                    select_value = Select.BLANK
                else:
                    select_value = options[0][1]
                widgets.append(
                    Select(
                        options,
                        id=f"field-{self.id}-{name}",
                        allow_blank=required,
                        value=select_value,
                    )
                )
            else:
                widgets.append(
                    Input(
                        value="" if default is None else str(default),
                        placeholder="required" if required else "",
                        id=f"field-{self.id}-{name}",
                    )
                )
        return widgets

    def read_config(self) -> ConverterConfig:  # NOSONAR
        """Read current field values and return a validated config instance.

        Raises:
            ValidationError: if required fields are missing or invalid.
        """
        if self._converter_type is None:
            raise RuntimeError("No converter type selected")
        model = get_config_model(self._converter_type)
        config_dict: dict = {}
        for name, fld in model.model_fields.items():
            field_type = _resolve_field_type(fld.annotation)
            widget_id = f"#field-{self.id}-{name}"
            if isinstance(field_type, type) and issubclass(field_type, enum.Enum):
                widget = self.query_one(widget_id, Select)
                if widget.value is not Select.BLANK:
                    config_dict[name] = field_type(widget.value)
            else:
                raw = self.query_one(widget_id, Input).value.strip()
                if raw:
                    target_type = (
                        fld.annotation.__origin__ if fld.annotation is not None and hasattr(fld.annotation, "__origin__") else fld.annotation
                    )
                    config_dict[name] = cast_value(raw, target_type)
        return model(**config_dict)  # type: ignore[return-value]

    def show_error(self, msg: str) -> None:
        """Display an error message in the panel's error Static widget."""
        self.query_one(self._ERROR_WIDGET_ID, Static).update(f"[red]{escape(msg)}[/red]")

    def clear_error(self) -> None:
        """Clear the panel's error Static widget."""
        self.query_one(self._ERROR_WIDGET_ID, Static).update("")
