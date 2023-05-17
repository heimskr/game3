#!/bin/bash

sudo pip3 install meson
sudo pip3 install ninja

mkdir .github-deps
cd .github-deps

wget https://download.gnome.org/sources/glib/2.76/glib-2.76.1.tar.xz
tar xf glib-2.76.1.tar.xz
cd glib-2.76.1
meson setup --libdir lib -Dtests=false meson_build .
cd meson_build
sudo ninja install
cd ..
meson setup --prefix $(realpath ../prefix) --libdir lib -Dtests=false meson_build_prefix .
cd meson_build_prefix
ninja install
cd ../..

wget https://download.gnome.org/sources/gtkmm/4.10/gtkmm-4.10.0.tar.xz
tar xf gtkmm-4.10.0.tar.xz
cd gtkmm-4.10.0
meson setup --prefix $(realpath ../prefix) --libdir lib -Dbuild-documentation=false -Dgtk4:media-gstreamer=disabled -Dgtk4:build-examples=false -Dgtk4:build-tests=false -Dgtk4:build-testsuite=false -Dgtk4:demos=false -Dwayland:documentation=false -Dwayland:tests=false -Dbuild-tests=false -Dbuild-demos=false -Dmaintainer-mode=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no -Dsigc++-3.0:build-documentation=false -Dsigc++-3.0:build-examples=false -Dsigc++-3.0:build-tests=false -Dsigc++-3.0:validation=false -Dsigc++-3.0:dist-warnings=no -Dsigc++-3.0:warnings=no -Dwayland-protocols:tests=false -Dmm-common:use-network=true meson_build .
cd meson_build
ninja install
