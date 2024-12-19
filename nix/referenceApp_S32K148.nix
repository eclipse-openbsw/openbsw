{ stdenv
, cmake
, gcc-arm-embedded
}:
stdenv.mkDerivation {
  name = "referenceApp";
  src = ../.;
  nativeBuildInputs = [
    cmake
    gcc-arm-embedded
  ];

  configurePhase = ''
    cmake -B cmake-build-s32k148 -S executables/referenceApp -DBUILD_TARGET_PLATFORM="S32K148EVB" --toolchain ../../admin/cmake/ArmNoneEabi.cmake
  '';
  buildPhase = ''
    cmake --build cmake-build-s32k148 --target app.referenceApp -j $NIX_BUILD_CORES
  '';
  installPhase = ''
    mkdir -p $out
    cp cmake-build-s32k148/application/app.referenceApp.elf $out/
  '';
}

