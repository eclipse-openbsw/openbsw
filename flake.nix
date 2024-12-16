{
  description = "Flake utils demo";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      {
        packages = {
          default = self.packages."${system}".referenceApp;

          referenceApp = pkgs.callPackage ./nix/referenceApp.nix {}; 
          interactiveIntegrationTests = self.checks."${system}".integrationTests.driverInteractive;
        };
        checks = {
          integrationTests = import ./nix/IntegrationTests.nix { inherit pkgs self system;};
        };
      }
    );
}
