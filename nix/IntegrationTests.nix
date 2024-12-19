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
      self.packages."${system}".udstool
    ] ++ (pkgs.callPackage ./pytestDependencies.nix {});

    environment.variables = {
      OPENBSW_SRC = ../.;
    };

    programs.bash.promptInit = ''
      PS1="$ "
    '';

  };

  testScript = { nodes, ... }: ''
    machine.wait_for_unit("default.target")
    machine.succeed("ifconfig vcan0")
    machine.succeed("ls $OPENBSW_SRC | grep README.md")
    machine.succeed("cd $OPENBSW_SRC/test/pyTest && pytest -s --target=posix_ci")
  '';

}
