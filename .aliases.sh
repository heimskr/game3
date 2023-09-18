#!/bin/bash

b() { b_next() { ninja; cd ..; }; cd builddir && b_next; }
alias reconf="meson setup --reconfigure builddir ."
