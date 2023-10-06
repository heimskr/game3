#!/bin/bash

OLD_DIR="$(pwd)"
cd "$2"
zig build-exe -O ReleaseSmall --main-pkg-path . src/resgen.zig
mv resgen "$OLD_DIR/resgen"
