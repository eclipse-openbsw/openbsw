from datetime import datetime

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
stylecheck = {"top_level_modulename": {"exclude": ["doc/index"]}}
dox_style_footer = "footer.yaml"
dox_trace_properties_file = "properties.yaml"

# Main page and files to be included and excluded
master_doc = "doc/index"
include_patterns = [
    "executables/**/doc/**",
    "libs/**/doc/**",
    "platforms/**/doc/**",
    "platforms/python/sca/doc/**",
    "tools/UdsTool/doc/**",
    "doc/**",
]
exclude_patterns = []
