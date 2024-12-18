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
          udstool = pkgs.callPackage ./nix/udstool.nix {};
          interactiveIntegrationTests = self.checks."${system}".integrationTests.driverInteractive;
        };
        checks = {
          integrationTests = import ./nix/IntegrationTests.nix { inherit pkgs self system;};
        };
        devShells.default = pkgs.mkShell {
          buildInputs = with self.packages."${system}"; [udstool]
            ++ pkgs.callPackage ./nix/pytestDependencies.nix {};
          inputsFrom = with self.packages."${system}"; [referenceApp];
        };
      }
    );
}
