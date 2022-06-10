# BUILD := release
ifeq ($(BUILD),release)
BUILDFLAGS := -Ofast -march=native
else
BUILDFLAGS := -g -O0
endif

ifeq ($(shell uname -s), Darwin)
LDFLAGS := -framework Cocoa -framework OpenGL -framework IOKit
else
LDFLAGS := -lGL
endif

DEPS         := eigen3 glm glfw3 libzstd gtk4 gtkmm-4.0 nlohmann_json
OUTPUT       := game3
COMPILER     ?= g++
CPPFLAGS     := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude -Istb
INCLUDES     := $(shell pkg-config --cflags $(DEPS))
LIBS         := $(shell pkg-config --libs   $(DEPS))
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread -Llibnoise/build/src -lnoise
SOURCES      := $(shell find src -name \*.cpp) src/resources.cpp
OBJECTS      := $(SOURCES:.cpp=.o)
RESXML       := $(OUTPUT).gresource.xml
CLOC_OPTIONS := . --exclude-dir=.vscode --fullpath --not-match-f='^.\/(src\/resources\.cpp|include\/resources\.h)$$'
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean test

all: $(OUTPUT)

src/resources.cpp: $(RESXML) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies $(RESXML))
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

%.o: %.cpp include/resources.h
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

include/resources.h: resources/buffered.frag resources/buffered.vert resources/rect.frag resources/rect.vert resources/sprite.frag resources/sprite.vert
	echo "#include <cstdlib>" > $@
	bin2c buffered_frag < resources/buffered.frag | tail -n +2 >> $@
	bin2c buffered_vert < resources/buffered.vert | tail -n +2 >> $@
	bin2c rectangle_frag < resources/rect.frag | tail -n +2 >> $@
	bin2c rectangle_vert < resources/rect.vert | tail -n +2 >> $@
	bin2c sprite_frag < resources/sprite.frag | tail -n +2 >> $@
	bin2c sprite_vert < resources/sprite.vert | tail -n +2 >> $@

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
	cloc $(CLOC_OPTIONS)

countbf:
	cloc --by-file $(CLOC_OPTIONS)

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
