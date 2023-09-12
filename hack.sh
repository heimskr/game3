#!/bin/bash

OLD_DIR="$(pwd)"
cd "$2"
zig build-obj -O ReleaseSmall --main-pkg-path . src/resources.zig
# echo "$OLD_DIR" >> /home/kai/Desktop/what.txt
mv resources.o "$OLD_DIR/src/resources.o"
ln -s vcpkg_installed "$OLD_DIR/vcpkg_installed"
