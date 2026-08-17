{
  stdenv,
  fetchFromGitHub,
  cmake,
  lib,
}:

# let
#   fastsimd = fetchFromGitHub {
#     owner = "Auburn";
#     repo = "FastSIMD";
#     rev = "16450dae9528727e500e7254f635a671f9c7ee2d";
#     hash = "sha256-86JqDMXbeK7Q0PgeFhKa7h2Jpl7Sa5tZNfyInhXF8tU=";
#   };
# in

stdenv.mkDerivation (finalAttrs: {
  pname = "fastnoise2";
  # version = "1.1.1";
  version = "0-unstable-2024-09-27";

  src = fetchFromGitHub {
    owner = "Auburn";
    repo = "FastNoise2";
    # tag = "v${finalAttrs.version}";
    # hash = "sha256-bmJ2iTmAAEYSZgsyLU8ZJsRnM2LH/4a2ITKGFJEDdvY=";
    rev = "9937723493bb6fd1b8f1491bf9ce251a8867226b";
    hash = "sha256-1JchJhg1Xn95mU5uMY1zv8kBNAlLOV9K+BrbFleZ46U=";
  };

  # postPatch = ''
  #   sed -i '/CPMAddPackage/,+4d' src/CMakeLists.txt
  #   sed -i '1i add_subdirectory(${fastsimd} fastsimd)' src/CMakeLists.txt
  # '';

  nativeBuildInputs = [
    cmake
  ];

  cmakeFlags = [
    # (lib.cmakeBool "FASTNOISE2_TOOLS" false)
    (lib.cmakeBool "FASTNOISE2_NOISETOOL" false)
  ];

  meta = {
    description = "Modular node graph based noise generation library using SIMD, C++17 and templates";
    license = lib.licenses.mit;
  };
})
