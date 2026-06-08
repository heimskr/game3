{
  stdenv,
  meson,
  ninja,
  cmake,
  pkg-config,
  flex,
  bison,
  opusfile,
  libvorbis,
  libxcb,
  libpng,
  curl,
  glibmm_2_68,
  freetype,
  libGL,
  libGLU,
  glfw3,
  asio,
  lz4,
  boost,
  libsigcxx30,
  leveldb,
  fastnoise2,
  glm,
  libbfd,
  libx11,
  libnoise,
  lib,
}:

stdenv.mkDerivation {
  pname = "game3";
  version = "1.0.0";

  src = ../.;

  postPatch = ''
    substituteInPlace src/meson.build \
      --replace-fail '-lnoise' '-lnoise-static'
  '';

  nativeBuildInputs = [
    meson
    ninja
    cmake
    pkg-config
    flex
    bison
  ];

  buildInputs = [
    opusfile
    libvorbis
    libxcb
    libpng
    curl
    glibmm_2_68
    freetype
    libGL
    libGLU
    glfw3
    asio
    lz4
    boost
    libsigcxx30
    leveldb
    fastnoise2
    libbfd
    libx11
    libnoise
    glm
  ];

  meta = {
    description = "the third game";
    mainProgram = "game3";
    license = lib.licenses.agpl3Plus;
    platforms = lib.platforms.linux;
  };
}
