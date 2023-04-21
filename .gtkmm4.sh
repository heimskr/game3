#!/bin/bash

mkdir ~/deps
cd ~/deps

wget https://download.gnome.org/sources/glib/2.76/glib-2.76.1.tar.xz
cd glib-2.76.1
meson setup --libdir lib meson_build .
cd meson_build
sudo ninja install
cd ../..

wget https://download.gnome.org/sources/gtkmm/4.10/gtkmm-4.10.0.tar.xz
tar xf gtkmm-4.10.0.tar.xz
cd gtkmm-4.10.0
meson setup --libdir lib -Dbuild-documentation=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dwayland:documentation=false -Dbuild-tests=false -Dbuild-demos=false -Dmaintainer-mode=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no meson_build .
cd meson_build
sudo ninja install
