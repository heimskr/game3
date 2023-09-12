#!/bin/bash

zig build-obj -O ReleaseSmall --main-pkg-path .. "$1"
mv resources.o src/resources.o