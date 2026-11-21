"""Shared Rich console instance for all flync_cli commands."""

import shutil

from rich.console import Console

# Rich falls back to a bare 80 columns when it cannot detect a real terminal (redirected output, CI logs,
# the test runner) - too narrow for the wider info reports (sockets, ip). Widen that fallback; a real
# terminal's actual size still wins.
_WIDTH, _ = shutil.get_terminal_size(fallback=(200, 50))
console = Console(force_terminal=True, width=_WIDTH)
