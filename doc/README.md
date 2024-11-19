# Building the project documentation

## Building documentation with [`Sphinx`](https://www.sphinx-doc.org/)

[`Download Zipped Documentation`](https://github.com/esrlabs/open-bsw/releases/download/preview/open-bsw-doc.zip)

Documentation in RST format throughout the code can be exported to HTML as described below.

Prerequisites:
- Python 3.10.x
- [`plantuml`](https://plantuml.com/starting)

First install the required Python modules:
```bash
pip install -r requirements.txt
```
and then build the documentation calling:
```bash
make html
```

## Building documentation with [`Doxygen`](https://www.doxygen.nl/)

Using the settings file ``doc/Doxyfile``,
`Doxygen` will generate documentation
from formatted comments in C/C++ code in the subdirectories...

* ``libs/bsp``
* ``libs/bsw``

by running the following in the ``doc`` directory...

```bash
doxygen Doxyfile
```

The entry point html file is found at `doc/doxygenOut/html/index.html`
