# BUILD := release
ifeq ($(BUILD),release)
BUILDFLAGS := -Ofast -march=native
else
BUILDFLAGS := -g -O0
endif

DEPS       := gl opengl stb eigen3 glm glfw3 libzstd gtk4 gtkmm-4.0
OUTPUT     := game3
COMPILER   ?= g++
CPPFLAGS   := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude
INCLUDES   := $(shell pkg-config --cflags $(DEPS)) -Inanovg/src
LIBS       := $(shell pkg-config --libs   $(DEPS))
LDFLAGS    := -L/lib $(LIBS) -lnanogui -pthread -lGLU -lglut -lnoise
SOURCES    := $(shell find src -name \*.cpp) src/resources.cpp
OBJECTS    := $(SOURCES:.cpp=.o)
RESXML     := $(OUTPUT).gresource.xml
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean test

all: $(OUTPUT)

src/resources.cpp: $(RESXML) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies $(RESXML))
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

%.o: %.cpp include/resources.h
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

include/resources.h: resources/tilemap.frag resources/tilemap.geom resources/tilemap.vert resources/buffered.frag resources/buffered.vert
	echo "#include <cstdlib>" > $@
	bin2c tilemap_frag < resources/tilemap.frag | tail -n +2 >> $@
	bin2c tilemap_geom < resources/tilemap.geom | tail -n +2 >> $@
	bin2c tilemap_vert < resources/tilemap.vert | tail -n +2 >> $@
	bin2c buffered_frag < resources/buffered.frag | tail -n +2 >> $@
	bin2c buffered_vert < resources/buffered.vert | tail -n +2 >> $@

$(OUTPUT): $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

clean:
	@ rm -f $(shell find src -name \*.o) $(OUTPUT) include/resources.h src/resources.cpp

count:
	cloc . --exclude-dir=.vscode

countbf:
	cloc --by-file . --exclude-dir=.vscode

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
