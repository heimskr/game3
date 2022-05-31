ifeq ($(BUILD),release)
BUILDFLAGS := -O3
else
BUILDFLAGS := -g -O0
endif

DEPS       := gl opengl stb eigen3 glm
OUTPUT     := game3
COMPILER   ?= g++
CPPFLAGS   := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude
INCLUDES   := $(shell pkg-config --cflags $(DEPS)) -Inanovg/src
LIBS       := $(shell pkg-config --libs   $(DEPS))
LDFLAGS    := -L/lib $(LIBS) -lnanogui -pthread -lGLU -lglut
SOURCES    := $(shell find src -name \*.cpp)
OBJECTS    := $(SOURCES:.cpp=.o)

.PHONY: all clean test

all: $(OUTPUT)

%.o: %.cpp include/resources.h
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

include/resources.h: resources/tilemap.frag resources/tilemap.geom resources/tilemap.vert
	echo "#include <cstdlib>" > $@
	bin2c tilemap_frag < resources/tilemap.frag | tail -n +2 >> $@
	bin2c tilemap_geom < resources/tilemap.geom | tail -n +2 >> $@
	bin2c tilemap_vert < resources/tilemap.vert | tail -n +2 >> $@

$(OUTPUT): $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

clean:
	@ rm -f $(shell find src -name \*.o) $(OUTPUT) include/resources.h

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
