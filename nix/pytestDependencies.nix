{ tmux
, can-utils
, python3
, fetchPypi
}:
# Dependencies required for Integration tests / pytest
[
  tmux
  can-utils
  (python3.withPackages (p: [
    p.pytest

    p.can-isotp# = 2.0.6
    p.exceptiongroup# = 1.2.2
    p.iniconfig# = 2.0.0
    p.msgpack# = 1.0.8
    p.packaging# = 24.1
    p.pluggy# = 1.5.0
    p.pyserial# = 3.5
    p.pytest# = 8.3.3
    p.python-can# = 4.4.2
    p.tomli# = 2.0.1
    p.typing-extensions# = 4.12.2
    # p.udsoncan# = 1.23.1
    (p.buildPythonPackage rec {
      pname = "udsoncan";
      version = "1.23.1";
      src = fetchPypi {
        inherit pname version;
        hash = "sha256-XlfzNk48aQpJ0Xsvdmj6VsNkr37J2PUT71gkFzMcpIA=";
      };
    })
    p.wrapt# = 1.16.0

  ]))
]
