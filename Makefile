LTO     :=
LDFLAGS :=

ifeq ($(BUILD),debug)
BUILDFLAGS := -g -O0
else ifeq ($(BUILD),tsan)
BUILDFLAGS := -g -O1 -fsanitize=thread
LDFLAGS    := -fsanitize=thread
else ifeq ($(BUILD),nonnative)
BUILDFLAGS := -Ofast
LTO        := -flto
else
BUILDFLAGS := -Ofast -march=native
LTO        := -flto
endif

ifeq ($(shell uname -s), Darwin)
LDFLAGS := $(LDFLAGS) -framework Cocoa -framework OpenGL -framework IOKit
else
LDFLAGS := $(LDFLAGS) -lGL
endif

DEPS         := glm glfw3 libzstd gtk4 gtkmm-4.0 glu
OUTPUT       := game3
COMPILER     ?= g++
CPPFLAGS     := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude -Ijson/include -Ieigen -Istb -Ilibnoise/src $(LTO)
ZIG          ?= zig
# --main-pkg-path is needed as otherwise it wouldn't let you embed any file outside of src/
ZIGFLAGS     := -O ReleaseSmall --main-pkg-path .
ifeq ($(GITHUB),true)
INCLUDES     := $(shell PKG_CONFIG_PATH=.github-deps/prefix/lib/pkgconfig pkg-config --cflags $(DEPS))
LIBS         := $(shell PKG_CONFIG_PATH=.github-deps/prefix/lib/pkgconfig pkg-config --libs   $(DEPS))
ZIG          := .zig/zig
else
INCLUDES     := $(shell pkg-config --cflags $(DEPS))
LIBS         := $(shell pkg-config --libs   $(DEPS))
endif
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread $(LTO)
SOURCES      := $(shell find src -name \*.cpp) src/gtk_resources.cpp
OBJECTS      := $(SOURCES:.cpp=.o) src/resources.o
RESXML       := $(OUTPUT).gresource.xml
CLOC_OPTIONS := . --exclude-dir=.vscode,libnoise,stb --fullpath --not-match-f='^.\/(src\/(gtk_)?resources\.cpp|include\/resources\.h)$$'
RESGEN       := ./resgen
NOISE_OBJ    := libnoise/src/libnoise.a

.PHONY: all clean flags test

all: $(NOISE_OBJ) $(OUTPUT)

submodules:
	git submodule update --init --recursive

$(NOISE_OBJ): submodules
	cd libnoise && cmake . && make

flags:
	@ echo "COMPILER: $(COMPILER)"
	@ echo
	@ echo "CFLAGS:   $(CFLAGS)"
	@ echo
	@ echo "LDFLAGS:  $(LDFLAGS)"

src/gtk_resources.cpp: $(RESXML) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies $(RESXML))
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

%.o: %.cpp include/resources.h
	@ printf "\e[2m[\e[22;32mc++\e[39;2m]\e[22m $< \e[2m$(strip $(BUILDFLAGS) $(LTO))\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

src/resources.o: src/resources.zig
	@ printf "\e[2m[\e[22;32mzig\e[39;2m]\e[22m $< \e[2m$(ZIGFLAGS)\e[22m\n"
	@ $(ZIG) build-obj $(ZIGFLAGS) $<  -femit-bin=$@

$(RESGEN): src/resgen.zig src/resources.zig
	@ printf "\e[2m[\e[22;32mzig\e[39;2m]\e[22m $< \e[2m$(ZIGFLAGS)\e[22m\n"
	@ $(ZIG) build-exe $(ZIGFLAGS) $<

include/resources.h: $(RESGEN)
	$(RESGEN) -h > $@

$(OUTPUT): $(OBJECTS) $(NOISE_OBJ)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

clean:
	@ rm -f $(shell find src -name \*.o) $(OUTPUT) src/gtk_resources.cpp include/resources.h $(RESGEN) $(RESGEN).o

count:
	cloc $(CLOC_OPTIONS)

countbf:
	cloc --by-file $(CLOC_OPTIONS)

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend: $(RESGEN) include/resources.h
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -Y -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	$(RESGEN) -d >> $(DEPFILE)
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)

libnoise/src/noise/basictypes.h: submodules
libnoise/src/noise/vectortable.h: submodules
libnoise/src/noise/noisegen.h: submodules
libnoise/src/noise/mathconsts.h: submodules
libnoise/src/noise/model/line.h: submodules
libnoise/src/noise/model/plane.h: submodules
libnoise/src/noise/model/cylinder.h: submodules
libnoise/src/noise/model/sphere.h: submodules
libnoise/src/noise/model/model.h: submodules
libnoise/src/noise/module/module.h: submodules
libnoise/src/noise/module/curve.h: submodules
libnoise/src/noise/module/min.h: submodules
libnoise/src/noise/module/invert.h: submodules
libnoise/src/noise/module/max.h: submodules
libnoise/src/noise/module/rotatepoint.h: submodules
libnoise/src/noise/module/spheres.h: submodules
libnoise/src/noise/module/ridgedmulti.h: submodules
libnoise/src/noise/module/clamp.h: submodules
libnoise/src/noise/module/const.h: submodules
libnoise/src/noise/module/power.h: submodules
libnoise/src/noise/module/cache.h: submodules
libnoise/src/noise/module/add.h: submodules
libnoise/src/noise/module/modulebase.h: submodules
libnoise/src/noise/module/abs.h: submodules
libnoise/src/noise/module/select.h: submodules
libnoise/src/noise/module/blend.h: submodules
libnoise/src/noise/module/billow.h: submodules
libnoise/src/noise/module/multiply.h: submodules
libnoise/src/noise/module/scalebias.h: submodules
libnoise/src/noise/module/cylinders.h: submodules
libnoise/src/noise/module/voronoi.h: submodules
libnoise/src/noise/module/terrace.h: submodules
libnoise/src/noise/module/exponent.h: submodules
libnoise/src/noise/module/displace.h: submodules
libnoise/src/noise/module/perlin.h: submodules
libnoise/src/noise/module/checkerboard.h: submodules
libnoise/src/noise/module/scalepoint.h: submodules
libnoise/src/noise/module/translatepoint.h: submodules
libnoise/src/noise/module/turbulence.h: submodules
libnoise/src/noise/latlon.h: submodules
libnoise/src/noise/interp.h: submodules
libnoise/src/noise/misc.h: submodules
libnoise/src/noise/exception.h: submodules
libnoise/src/noise/noise.h: submodules
libnoise/noiseutils/noiseutils.h: submodules
