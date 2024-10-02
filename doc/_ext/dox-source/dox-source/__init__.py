from docutils import nodes
import os.path
from sphinx.directives.code import LiteralInclude

from sphinx.util import logging

logger = logging.getLogger(__name__)


def source_role(_role, rawtext, text, lineno, inliner, _options={}, _content=[]):
    rst_file = inliner.document.settings.env.docname
    split_rst_file = rst_file.replace("\\", "/").split("/doc/")
    file = None
    if len(split_rst_file) > 1:  # source referenced from regular module
        path2mod = "../" * (split_rst_file[-1].count("/") + 1)
        file = os.path.join(
            os.path.dirname(inliner.document.current_source), path2mod + text
        )
    if not file or not os.path.isfile(str(file)):  # fallback for non-standard modules
        file = os.path.join(os.path.dirname(inliner.document.current_source), text)
    if not os.path.isfile(str(file)):
        logger.error(
            '%s.rst:%d: file "%s" not found'
            % (inliner.document.settings.env.docname, lineno, text)
        )

    node = nodes.literal(rawtext, text)
    node.set_class("highlight")
    node.set_class("highlight-source")

    return [node], []


class SourceInclude(LiteralInclude):
    def run(self):
        if "caption" in self.options and not self.options["caption"]:
            self.options["caption"] = self.arguments[0]

        split_docname = self.env.docname.replace("\\", "/").split("/doc/")
        if len(split_docname) > 1:
            path2mod = "../" * (split_docname[-1].count("/") + 1)
            self.arguments[0] = path2mod + self.arguments[0]

        return super().run()


def setup(app):
    app.add_role("source", source_role)
    app.add_directive("sourceinclude", SourceInclude)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
