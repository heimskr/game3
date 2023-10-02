#!/bin/bash

b() { b_next() { ninja; cd ..; }; cd builddir && b_next; }
alias reconf="meson setup --reconfigure builddir ."
alias ts="./builddir/src/game3 -s"
alias t="./builddir/src/game3"
bts() { b && ts; }
bt() { b && t; }
ta() { ./builddir/src/game3 -s 12256 world.alt.db; }
bta() { b && ta; }