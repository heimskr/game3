#!/bin/sh

# I realize this filename sounds incredibly suspicious but this script is entirely harmless.
# All it does is list all .cpp files in the `discord` directory at the root of the repo.
# This is used during `meson setup`.

find ../discord -name '*.cpp'
