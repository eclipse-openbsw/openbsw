{ python3Packages
, fetchPypi
}:
python3Packages.buildPythonApplication  {
  name = "udstool";
  src = ../tools/UdsTool;

  build-system = [ python3Packages.setuptools ];
  dependencies = with python3Packages; [
    can-isotp
    click
    (buildPythonPackage rec {
      pname = "doipclient";
      version = "1.1.1";
      src = fetchPypi {
        inherit pname version;
        hash = "sha256-U9AtlB8T13+wrYPi4B8QY+LBvib0lXEOmiVcD60EW4Y=";
      };
    })
    python-can
    (buildPythonPackage rec {
      pname = "udsoncan";
      version = "1.23.1";
      src = fetchPypi {
        inherit pname version;
        hash = "sha256-XlfzNk48aQpJ0Xsvdmj6VsNkr37J2PUT71gkFzMcpIA=";
      };
    })
  ];
}
