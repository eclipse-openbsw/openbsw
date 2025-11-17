from datetime import datetime
import yaml
from docutils import nodes

# General configuration
project = "Eclipse OpenBSW Documentation"
copyright = f"{datetime.now().year} Accenture"

extensions = [
    "sphinxcontrib.jquery",
    "sphinxcontrib.plantuml",
    "sphinx.ext.todo",
    "sphinx_copybutton",
    "sphinx.ext.graphviz",
    "dox_style",
    "dox_util",
    "dox_trace",
]

# Extensions configuration
html_context = {"data_classification_default": None}
stylecheck = {"top_level_modulename": {"exclude": ["doc/dev/index", 'tools/**']}}

dox_style_footer = "footer.yaml"
dox_trace_properties_file = "properties.yaml"

# Main page and files to be included and excluded
master_doc = "doc/dev/index"
include_patterns = [
    "executables/**/doc/**",
    "libs/**/doc/**",
    "platforms/**/doc/**",
    "platforms/python/sca/doc/**",
    "tools/**",
    "doc/dev/**",
]
exclude_patterns = []

def replace_placeholders(app, doctree, docname):
    with open("properties.yaml", "r") as f:
        props = yaml.safe_load(f)

    gcc_arm = props["tool"].get("gcc-arm-none-eabi", "x.x")
    llvm_arm = props["tool"].get("llvm-arm", "x.x")
    ubuntu_version = props["tool"].get("ubuntu_version", "x.x")

    for node in doctree.traverse(nodes.Text):
        text = node.astext()
        new_text = text

        if "gcc-arm-none-eabi" in text and "x.x" in text:
            new_text = new_text.replace("x.x", gcc_arm)

        if "LLVM-ET-Arm" in text and "x.x" in text:
            new_text = new_text.replace("x.x", llvm_arm)

        elif "Ubuntu-x.x" in text:
            new_text = new_text.replace("x.x", ubuntu_version)

        if new_text != text:
            node.parent.replace(node, nodes.Text(new_text))


def setup(app):
    # Doctree-resolved is a single event that is emitted when the document tree has been fully resolved
    app.connect("doctree-resolved", replace_placeholders)