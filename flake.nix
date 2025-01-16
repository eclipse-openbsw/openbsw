{
  description = "OpenBSW: A Code-first Software Platform for Automotive Microcontrollers";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    treefmt.url = "github:numtide/treefmt/v2.1.0";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      treefmt,
    }:
    (import "${nixpkgs}/lib/default.nix").attrsets.recursiveUpdate

      (flake-utils.lib.eachSystem
        (with flake-utils.lib.system; [
          aarch64-linux
          x86_64-linux
        ])
        (
          system:
          let
            pkgs = nixpkgs.legacyPackages.${system};
          in
          {
            packages = {
              default = self.packages."${system}".referenceApp;

              referenceApp = pkgs.callPackage ./nix/referenceApp.nix { };
            };
            devShells.default = import ./nix/shell.nix {
              inherit pkgs self system;
              treefmt = treefmt.packages."${system}".default;
            };
          }
        )
      )

      (
        flake-utils.lib.eachSystem
          (with flake-utils.lib.system; [
            aarch64-linux
            x86_64-linux
            aarch64-darwin
            x86_64-darwin
          ])
          (
            system:
            let
              pkgs = nixpkgs.legacyPackages.${system};
            in
            {
              packages = {
                referenceApp_S32K148 = pkgs.callPackage ./nix/referenceApp_S32K148.nix { };
                udstool = pkgs.callPackage ./nix/udstool.nix { };
                interactiveIntegrationTests = self.checks."${system}".integrationTests.driverInteractive;

                # re-export of formatters
                treefmt = treefmt.packages."${system}".default;
                nixfmt = pkgs.nixfmt-rfc-style;
                clang-tools = pkgs.llvmPackages_17.clang-tools;
                cmake-format = pkgs.cmake-format;
              };
              checks = {
                integrationTests = import ./nix/IntegrationTests.nix { inherit pkgs self system; };
              };
            }
          )
      );
}
