#!/usr/bin/bash

sudo pip3 install meson
sudo pip3 install ninja

mkdir .github-deps
cd .github-deps

ORIG_DIR=$(pwd)

# wget https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
# tar xf pkg-config-0.29.2.tar.gz
# cd pkg-config-0.29.2
# ./configure --prefix=/usr --with-internal-glib
# make -j2
# sudo make install
# cd "$ORIG_DIR"
export PKG_CONFIG_PATH="$(realpath prefix/lib/pkgconfig):$PKG_CONFIG_PATH:/usr/lib/pkgconfig:/usr/share/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig"

# wget https://download.gnome.org/sources/glib/2.77/glib-2.77.0.tar.xz
# tar xf glib-2.77.0.tar.xz
# cd glib-2.77.0
# # meson setup --prefix /usr --libdir lib -Dtests=false meson_build .
# # cd meson_build
# # sudo ninja install
# # cd ..
# meson setup --prefix $(realpath ../prefix) --libdir lib -Dtests=false meson_build_prefix .
# cd meson_build_prefix
# ninja install
# cd "$ORIG_DIR"

# wget https://github.com/silnrsi/graphite/archive/refs/heads/master.zip
# unzip master.zip
# cd graphite-master
# cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr
# make -j2
# sudo make install
# cd "$ORIG_DIR"

# wget https://download.gnome.org/sources/pango/1.51/pango-1.51.0.tar.xz
# tar xf pango-1.51.0.tar.xz
# cd pango-1.51.0
# patch < ../../.pango.patch
# meson setup --prefix $(realpath ../prefix) --libdir lib meson_build .
# cd meson_build
# ninja install
# cd "$ORIG_DIR"

# wget https://download.gnome.org/sources/gtk/4.12/gtk-4.12.0.tar.xz
# tar xf gtk-4.12.0.tar.xz
# cd gtk-4.12.0
# sed -i "s/graphene_dep,/graphene_dep,dependency('graphite2'),/" gtk/meson.build
# meson setup --prefix $(realpath ../prefix) --libdir lib -Dmedia-gstreamer=disabled -Dbuild-examples=false -Dbuild-tests=false -Dbuild-testsuite=false -Dbuild-demos=false -Dwayland:documentation=false -Dwayland:tests=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no -Dsigc++-3.0:build-documentation=false -Dsigc++-3.0:build-examples=false -Dsigc++-3.0:build-tests=false -Dsigc++-3.0:validation=false -Dsigc++-3.0:dist-warnings=no -Dsigc++-3.0:warnings=no -Dwayland-protocols:tests=false -Dmm-common:use-network=true meson_build .
# cd meson_build
# ninja install
# cd ..
# meson setup --reconfigure --prefix /usr --libdir lib -Dmedia-gstreamer=disabled -Dbuild-examples=false -Dbuild-tests=false -Dbuild-testsuite=false -Dbuild-demos=false -Dwayland:documentation=false -Dwayland:tests=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no -Dsigc++-3.0:build-documentation=false -Dsigc++-3.0:build-examples=false -Dsigc++-3.0:build-tests=false -Dsigc++-3.0:validation=false -Dsigc++-3.0:dist-warnings=no -Dsigc++-3.0:warnings=no -Dwayland-protocols:tests=false -Dmm-common:use-network=true meson_build .
# cd meson_build
# sudo ninja install
# cd "$ORIG_DIR"

wget https://download.gnome.org/sources/gtkmm/4.12/gtkmm-4.12.0.tar.xz
tar xf gtkmm-4.12.0.tar.xz
cd gtkmm-4.12.0
sed -i "s/, glibmm_generate_extra_defs_dep/, glibmm_generate_extra_defs_dep, dependency('graphite2', required: true)/" tools/extra_defs_gen/meson.build
sed -i "s/, pangomm_dep/, pangomm_dep, dependency('graphite2', required: true)/" meson.build
meson setup --prefix $(realpath ../prefix) --libdir lib -Dbuild-documentation=false -Dgtk4:media-gstreamer=disabled -Dgtk4:build-examples=false -Dgtk4:build-tests=false -Dgtk4:build-testsuite=false -Dgtk4:demos=false -Dwayland:documentation=false -Dwayland:tests=false -Dbuild-tests=false -Dbuild-demos=false -Dmaintainer-mode=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no -Dsigc++-3.0:build-documentation=false -Dsigc++-3.0:build-examples=false -Dsigc++-3.0:build-tests=false -Dsigc++-3.0:validation=false -Dsigc++-3.0:dist-warnings=no -Dsigc++-3.0:warnings=no -Dwayland-protocols:tests=false -Dmm-common:use-network=true meson_build . || true
sed -i "s/requires: \\['gtk4',/libraries: ['gtk4',/" subprojects/gtk/meson.build
meson setup --prefix $(realpath ../prefix) --libdir lib -Dbuild-documentation=false -Dgtk4:media-gstreamer=disabled -Dgtk4:build-examples=false -Dgtk4:build-tests=false -Dgtk4:build-testsuite=false -Dgtk4:demos=false -Dwayland:documentation=false -Dwayland:tests=false -Dbuild-tests=false -Dbuild-demos=false -Dmaintainer-mode=false -Dlibsigcplusplus:build-documentation=false -Dlibsigcplusplus:build-examples=false -Dlibsigcplusplus:build-tests=false -Dlibsigcplusplus:validation=false -Dlibsigcplusplus:dist-warnings=no -Dlibsigcplusplus:warnings=no -Dsigc++-3.0:build-documentation=false -Dsigc++-3.0:build-examples=false -Dsigc++-3.0:build-tests=false -Dsigc++-3.0:validation=false -Dsigc++-3.0:dist-warnings=no -Dsigc++-3.0:warnings=no -Dwayland-protocols:tests=false -Dmm-common:use-network=true meson_build .
cd meson_build
ninja install --verbose
