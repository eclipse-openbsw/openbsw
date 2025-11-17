# Building the Project Documentation

## Building Developer Documentation With [`Sphinx`](https://www.sphinx-doc.org/)

Sphinx is a documentation generator originally created for the Python language, but now used widely
for many kinds of projects including C++.

Sphinx uses
[`reStructuredText`](https://docutils.sourceforge.io/rst.html) as its markup language.
If you are not familiar with reStructuredText, you may read the
[`reStructuredText Primer`](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)
on the Sphinx webpage first.

Prerequisites:
- Install Python 3.10 or higher
- Install [`plantuml`](https://plantuml.com/starting)
- Install the required Python modules:
  ```bash
  pip install -r doc/dev/requirements.txt
  ```

To build the documentation, go to `doc/dev` and call:
```bash
make html
```

The entry point HTML page is found at `doc/dev/build/html/doc/dev/index.html`.

## Building API Documentation with [`Doxygen`](https://www.doxygen.nl/)

 Using the settings file `doc/api/Doxyfile`, Doxygen generates the documentation from formatted
comments in the C/C++ code across all relevant subfolders; see
[Doxygen Main Page](DoxygenMainPage.md) for details.

To build the documentation, go to `doc/api` and call:
```bash
doxygen Doxyfile
```

The entry point HTML page is found at `doc/api/doxygenOut/html/index.html`.

XML output is also generated in  `doc/api/doxygenOut/xml/` from which coverage information can
be extracted using [`coverxygen`](https://github.com/psycofdj/coverxygen). For example:

```bash
$ python3 -m coverxygen --format summary --xml-dir doxygenOut/xml/ --src-dir ../.. --output -
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
