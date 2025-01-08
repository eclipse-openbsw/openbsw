{
  stdenv,
  cmake,
}:
stdenv.mkDerivation {
  name = "referenceApp";
  src = ../.;
  nativeBuildInputs = [
    cmake
  ];

  configurePhase = ''
    cmake -B cmake-build-posix -S executables/referenceApp
  '';
  buildPhase = ''
    cmake --build cmake-build-posix --target app.referenceApp -j $NIX_BUILD_CORES
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp cmake-build-posix/application/app.referenceApp.elf $out/bin/referenceApp
  '';
  separateDebugInfo = true;
  hardeningDisable = ["all"];
}
