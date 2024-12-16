{ pkgs
, self
, system
}:
pkgs.nixosTest {
  name = "referenceApp-integration-test";

  nodes.machine = { config, pkgs, ... }: {
    system.stateVersion = "24.11";
    boot.loader.systemd-boot.enable = true;
    boot.loader.efi.canTouchEfiVariables = true;

    boot.kernelModules = [ "vcan" ];

    systemd.network.enable = true;
    systemd.network.wait-online.enable = false; # vm is isolated
    networking.useDHCP = false;
    systemd.network.netdevs.vcan0 = {
      enable = true;
      netdevConfig = {
        Name = "vcan0";
        Kind = "vcan";
      };
      extraConfig = "[vcan]";
    };
    systemd.network.networks.vcan0 = {
      enable = true;
      matchConfig = {
        Name = "vcan0";
      };
    };

    environment.systemPackages = [
      self.packages."${system}".referenceApp
      pkgs.can-utils
      (pkgs.python3.withPackages (p: [
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
        (pkgs.python3Packages.buildPythonPackage rec {
          pname = "udsoncan";
          version = "1.23.1";
          src = pkgs.fetchPypi {
            inherit pname version;
            hash = "sha256-XlfzNk48aQpJ0Xsvdmj6VsNkr37J2PUT71gkFzMcpIA=";
          };
        })
        p.wrapt# = 1.16.0

      ]))
    ];

    environment.variables = {
      OPENBSW_SRC = ../.;
    };

  };

  testScript = { nodes, ... }: ''
    machine.wait_for_unit("default.target")
    machine.succeed("ifconfig vcan0")
    machine.succeed("ls $OPENBSW_SRC | grep README.md")
    machine.succeed("cd $OPENBSW_SRC/test/pyTest && pytest --target=posix")
  '';

}
