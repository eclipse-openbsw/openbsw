{
  pkgs,
  self,
  system,
}:
pkgs.testers.runNixOSTest {
  name = "referenceApp-integration-test";

  nodes.machine =
    { config, pkgs, ... }:
    {
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
      ] ++ (pkgs.callPackage ./pytestDependencies.nix { });

      environment.variables = {
        OPENBSW_SRC = ../.;
      };

      # required by test_udsToolRDBI.py as long at it's using tmux
      programs.bash.promptInit = ''
        PS1="$ "
      '';

    };

  testScript =
    { nodes, ... }: # python
    ''
      machine.wait_for_unit("default.target")
      machine.succeed("ifconfig vcan0")
      virt = machine.succeed("systemd-detect-virt").strip()
      if virt != 'kvm':
        # if your pc has kvm enabled (/dev/kvm exists) but it is not used in 'nix flake check'
        # the nixbld users probably lack access. To fix this you could
        # add all the nixbld users to the kvm group with the following command
        #
        # for u in $(getent group nixbld | cut -d: -f4 | tr "," "\n"); do sudo usermod -a -G kvm $u; done
        raise Exception(f"Tests need to be run with kvm. Found {virt} instead")
      machine.succeed("ls $OPENBSW_SRC | grep README.md")
      machine.succeed("cd $OPENBSW_SRC/test/pyTest && pytest -s --target=posix_ci")
    '';

}
