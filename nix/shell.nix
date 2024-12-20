{
  pkgs,
  self,
  system,
  treefmt,
}:
pkgs.mkShell {
  inputsFrom = with self.packages."${system}"; [
    referenceApp
    referenceApp_S32K148
  ];

  packages =
    with self.packages."${system}";
    [ udstool ]
    ++ pkgs.callPackage ./pytestDependencies.nix { }
    ++ (with pkgs; [
      gdb
      llvmPackages_17.clang-tools
      minicom
      black
      cmake-format
      nixfmt-rfc-style
      treefmt
    ]);

}
