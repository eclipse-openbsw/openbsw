import os
import warnings
import importlib.util

from pathlib import Path
from datetime import datetime
import yaml
from yaml.loader import SafeLoader

from .label_check import *

logger = logging.getLogger(__name__)


def config_inited(app, config):
    # suppress deprecated warnings in some Sphinx extensions
    warnings.filterwarnings(
        "ignore", category=DeprecationWarning, module="sphinxcontrib"
    )

    # page footer
    if config.dox_style_footer:
        if os.path.isfile(config.dox_style_footer):
            with open(config.dox_style_footer, "r") as f:
                config["footer_string"] = yaml.load(f, Loader=SafeLoader)

    # HTML theme and options
    config.html_theme = "sphinx_rtd_theme"
    config.html_show_copyright = False
    config.html_show_sphinx = False
    config.html_show_sourcelink = True
    config.html_style = "rtd_theme_overrides.css"
    config.html_theme_options.update(
        {
            "style_external_links": True,
            "sticky_navigation": False,
            "prev_next_buttons_location": None,
            "navigation_depth": 6,
        }
    )

    # HTML additional files
    static = Path.joinpath(Path(__file__).parent, "_static").as_posix()
    config.html_static_path = [static]
    config.templates_path = [static]
    app.add_css_file("colors.css")
    app.add_css_file("colored_table.css")
    app.add_css_file("header.css")

    # data classification
    if "data_classification_default" not in config.html_context:
        config.html_context["data_classification_default"] = "Confidential"

    # latexpdf needs to know who certain unicode characters shall be rendered
    if not config.latex_elements:
        config.latex_elements = {}
    if not "preamble" in config.latex_elements:
        config.latex_elements["preamble"] = ""
    config.latex_elements[
        "preamble"
    ] += r"""
        \DeclareUnicodeCharacter{25BA}{$\blacktriangleright$}
        \DeclareUnicodeCharacter{25BC}{$\blacktriangledown$}
        \DeclareUnicodeCharacter{25B2}{$\blacktriangle$}
        \DeclareUnicodeCharacter{25C4}{$\blacktriangleleft$}
        """
    if not "extrapackages" in config.latex_elements:
        config.latex_elements["extrapackages"] = ""
    config.latex_elements[
        "extrapackages"
    ] += r"""
        \usepackage{pmboxdraw}
        """

    # overwrite PlantUML path, needed for Windows
    if "PLANTUML_DIR" in os.environ:
        config.plantuml = "java -Xlog:disable -jar {}".format(
            Path(os.getenv("PLANTUML_DIR")).joinpath("plantuml.jar").resolve()
        )

    # global definitions (important: do not use leading spaces)
    if not config.rst_prolog:
        config.rst_prolog = ""
    config.rst_prolog += """
.. role:: green
.. role:: yellow
.. role:: orange
.. role:: red
.. role:: blue
.. role:: white
.. role:: black
.. role:: grey
.. role:: greyitalic
.. role:: raw-html(raw)
    :format: html
.. |br| replace:: :raw-html:`</br>`
"""

    # case-insensitive glossary
    if "ref.term" not in config.suppress_warnings:
        config.suppress_warnings.append("ref.term")

    ################

    # At the end try to load a custom specific configuration file
    # which can be used to overwrite all values set by this extension.
    try:
        extend_filename = Path(app.confdir) / "extend_conf.py"
        if os.path.exists(extend_filename):
            spec = importlib.util.spec_from_file_location(
                "extend_conf", extend_filename
            )
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            module.extend_conf(config)
            print("Custom configuration file loaded")
    except (ImportError, TypeError):
        logger.warning(
            "extend_conf.py requires a function\n    def extend_conf(config: dict)"
        )


def get_footer_string(app, docname):
    docpath = docname
    while True:
        if docpath in app.config["footer_string"]:
            return app.config["footer_string"][docpath]
        else:
            pos = docpath.rfind("/")
            if pos >= 0:
                docpath = docpath[:pos]
            elif docpath != ".":
                docpath = "."
            else:
                return None


def add_extrafooter(app, _doctree, docname):
    build_type = (
        "official build" if "OFFICIAL_BUILD" in os.environ else "UNOFFICIAL BUILD"
    )
    build_time = datetime.today().strftime("%Y-%m-%d %H:%M:%S")
    app.env.metadata[docname]["footer_string"] = (
        "Documentation from " + build_time + ", " + build_type + "."
    )

    if hasattr(app.config, "footer_string"):
        ftext = get_footer_string(app, docname)
        if ftext:
            app.env.metadata[docname]["footer_string"] += "<br>" + ftext


def setup(app):
    # Must be called before any other extension.
    # Priority 0 is the highest priority.
    app.connect("config-inited", config_inited, priority=0)

    app.connect("doctree-resolved", add_extrafooter)

    app.connect("env-before-read-docs", prepare_labels)
    app.connect("env-purge-doc", purge_labels)
    app.connect("doctree-read", collect_labels)
    app.connect("env-merge-info", merge_labels_parallel_build)

    app.add_config_value("dox_style_footer", None, "html")

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
