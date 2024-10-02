import sys
from pathlib import Path
from datetime import datetime

# General configuration
project = "Eclipse OpenBSW Documentation"
copyright = f"Copyright (c) {datetime.now().year} Accenture"

# Extensions
EXTENSION_ROOT = Path("_ext")
sys.path.extend([str(path.resolve()) for path in EXTENSION_ROOT.glob("*")])

extensions = [
    "sphinxcontrib.jquery",
    "sphinxcontrib.plantuml",
    "sphinx.ext.todo",
    "sphinx_copybutton",
    "sphinx.ext.graphviz",
    "dox-source",
    "dox-rule",
    "dox-config",
]

# Extensions configuration
html_context = {"data_classification_default": None}
stylecheck = {"top_level_modulename": {"exclude": ["doc/index"]}}
dox_style_footer = "footer.yaml"

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
