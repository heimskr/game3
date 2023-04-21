#!/bin/bash

mkdir .github-deps
cd .github-deps

wget https://download.gnome.org/sources/glib/2.76/glib-2.76.1.tar.xz
tar xf glib-2.76.1.tar.xz
cd glib-2.76.1
meson setup --prefix $(realpath ../prefix) --libdir lib meson_build .
cd meson_build
ninja install
cd ../..

wget https://download.gnome.org/sources/gtkmm/4.10/gtkmm-4.10.0.tar.xz
tar xf gtkmm-4.10.0.tar.xz
cd gtkmm-4.10.0
meson setup --prefix $(realpath ../prefix) --libdir lib -Dbuild-documentation=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dwayland:documentation=false -Dbuild-tests=false -Dbuild-demos=false -Dmaintainer-mode=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no meson_build .
cd meson_build
ninja install
