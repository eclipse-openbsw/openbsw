{
  tmux,
  can-utils,
  python3,
  fetchPypi,
}:
# Dependencies required for Integration tests / pytest
[
  tmux
  can-utils
  (python3.withPackages (p: [
    p.pytest

    p.can-isotp
    p.exceptiongroup
    p.iniconfig
    p.msgpack
    p.packaging
    p.pluggy
    p.pyserial
    p.pytest
    p.python-can
    p.tomli
    p.typing-extensions
    (p.buildPythonPackage rec {
      pname = "udsoncan";
      version = "1.23.1";
      src = fetchPypi {
        inherit pname version;
        hash = "sha256-XlfzNk48aQpJ0Xsvdmj6VsNkr37J2PUT71gkFzMcpIA=";
      };
    })
    p.wrapt

  ]))
]
