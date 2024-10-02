# Building the project documentation

[`Documentation`](https://github.com/esrlabs/open-bsw/releases/download/preview/open-bsw-doc.zip)
The documentation is located under ``doc`` folder in RST format and can be exported to HTML as
described below.
Pre-requisites:
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