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
          referenceApp_S32K148 = pkgs.callPackage ./nix/referenceApp_S32K148.nix {};
          udstool = pkgs.callPackage ./nix/udstool.nix {};
          interactiveIntegrationTests = self.checks."${system}".integrationTests.driverInteractive;
        };
        checks = {
          integrationTests = import ./nix/IntegrationTests.nix { inherit pkgs self system;};
        };
        devShells.default = pkgs.mkShell {
          packages = with self.packages."${system}"; [udstool]
            ++ pkgs.callPackage ./nix/pytestDependencies.nix {}
            ++ (with pkgs; [ gdb ]);
          inputsFrom = with self.packages."${system}"; [referenceApp referenceApp_S32K148];
        };
      }
    );
}
