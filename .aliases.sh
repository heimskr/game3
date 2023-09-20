#!/bin/bash

b() { b_next() { ninja; cd ..; }; cd builddir && b_next; }
alias reconf="meson setup --reconfigure builddir ."
alias ts="./builddir/src/game3 -s"
alias t="./builddir/src/game3"
