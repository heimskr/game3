#!/bin/bash

b() { cd builddir && { ninja; cd .. } }
alias reconf="CXX='ccache clang++' CXX_LD='mold' meson setup --reconfigure builddir ."
alias ts="./builddir/src/game3 -s"
alias t="./builddir/src/game3"
bts() { b && ts; }
bt() { b && t; }
