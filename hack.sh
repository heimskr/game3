#!/bin/bash

OLD_DIR="$(pwd)"
cd "$2"
zig build-obj -O ReleaseSmall --main-pkg-path . src/resources.zig
mv resources.o "$OLD_DIR/src/resources.o"
