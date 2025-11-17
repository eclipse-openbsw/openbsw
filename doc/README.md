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

Using the settings file ``doc/api/Doxyfile``,
`Doxygen` will generate documentation
from formatted comments in C/C++ code in the subdirectories...

* ``libs/bsp``
* ``libs/bsw``

by running the following in the ``doc/api`` directory...

```bash
doxygen Doxyfile
```

The entry point html file is found at `doc/api/doxygenOut/html/index.html`

XML output is also generated in  `doc/api/doxygenOut/xml/` from which coverage information can be extracted using [`coverxygen`](https://github.com/psycofdj/coverxygen). For example...

```bash
$ python3 -m coverxygen --format summary --xml-dir doxygenOut/xml/ --src-dir .. --output -
Classes    :  50.2% (231/460)
Defines    :   2.4% (4/169)
Enum Values:  35.9% (185/515)
Enums      :  41.7% (45/108)
Files      :   7.5% (47/630)
Friends    :   9.8% (5/51)
Functions  :  41.9% (1826/4354)
Namespaces :   4.3% (5/116)
Pages      : 100.0% (3/3)
Structs    :  20.8% (55/264)
Typedefs   :  29.0% (226/780)
Unions     : 100.0% (1/1)
Variables  :   7.7% (110/1435)
-----------------------------------
Total      :  30.9% (2743/8886)

```