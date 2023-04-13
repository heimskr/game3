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

DEPS         := eigen3 glm glfw3 libzstd gtk4 gtkmm-4.0 nlohmann_json glu
OUTPUT       := game3
COMPILER     ?= g++
CPPFLAGS     := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude -Istb -Ilibnoise/src
INCLUDES     := $(shell pkg-config --cflags $(DEPS))
LIBS         := $(shell pkg-config --libs   $(DEPS))
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread -Llibnoise/build/src -lnoise
SOURCES      := $(shell find src -name \*.cpp) src/gtk_resources.cpp
OBJECTS      := $(SOURCES:.cpp=.o)
RESXML       := $(OUTPUT).gresource.xml
CLOC_OPTIONS := . --exclude-dir=.vscode,libnoise,stb --fullpath --not-match-f='^.\/(src\/(gtk_)?resources\.cpp|include\/resources\.h)$$'
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean flags test

all: $(OUTPUT)

flags:
	@ echo "COMPILER: $(COMPILER)"
	@ echo
	@ echo "CFLAGS:   $(CFLAGS)"
	@ echo
	@ echo "LDFLAGS:  $(LDFLAGS)"

src/gtk_resources.cpp: $(RESXML) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies $(RESXML))
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

%.o: %.cpp
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

src/resources.o: src/resources.zig resources/buffered.frag resources/buffered.vert resources/rectangle.frag resources/rectangle.vert resources/sprite.frag resources/sprite.vert resources/blur.frag resources/blur.vert resources/reshader.vert resources/multiplier.frag
	@ printf "\e[2m[\e[22;32mzig\e[39;2m]\e[22m $<\n"
	@ zig build-obj $< --main-pkg-path . -femit-bin=$@ -O ReleaseSmall

$(OUTPUT): $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

clean:
	@ rm -f $(shell find src -name \*.o) $(OUTPUT) src/gtk_resources.cpp

count:
	cloc $(CLOC_OPTIONS)

countbf:
	cloc --by-file $(CLOC_OPTIONS)

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -Y -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
