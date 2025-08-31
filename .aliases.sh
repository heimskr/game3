#!/bin/bash

BUILDDIR="${GAME3_BUILDDIR:-builddir}"
b() { cd $BUILDDIR && { ninja -j$(($(nproc)-4)); cd ..; }; }
alias reconf="CXX='ccache clang++' CC='ccache clang' CXX_LD='mold' meson setup --reconfigure $BUILDDIR ."
alias ts="./$BUILDDIR/src/game3 -s"
alias t="./$BUILDDIR/src/game3"
bts() { b && ts; }
bt() { b && t; }

