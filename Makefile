# BUILD := release
LTO :=
LDFLAGS :=
ifeq ($(BUILD),release)
BUILDFLAGS := -Ofast -march=native
LTO := -flto
else ifeq ($(BUILD),tsan)
BUILDFLAGS := -g -O1 -fsanitize=thread
LDFLAGS := -fsanitize=thread
else
BUILDFLAGS := -g -O0
endif

ifeq ($(shell uname -s), Darwin)
LDFLAGS := $(LDFLAGS) -framework Cocoa -framework OpenGL -framework IOKit
else
LDFLAGS := $(LDFLAGS) -lGL
endif

DEPS         := eigen3 glm glfw3 libzstd gtk4 gtkmm-4.0 nlohmann_json glu
OUTPUT       := game3
COMPILER     ?= g++
CPPFLAGS     := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude -Istb -Ilibnoise/src $(LTO)
ZIG          ?= zig
# --main-pkg-path is needed as otherwise it wouldn't let you embed any file outside of src/
ZIGFLAGS     := -O ReleaseSmall --main-pkg-path .
INCLUDES     := $(shell pkg-config --cflags $(DEPS))
LIBS         := $(shell pkg-config --libs   $(DEPS))
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread -Llibnoise/build/src -lnoise $(LTO)
SOURCES      := $(shell find src -name \*.cpp) src/gtk_resources.cpp
OBJECTS      := $(SOURCES:.cpp=.o) src/resources.o
RESXML       := $(OUTPUT).gresource.xml
CLOC_OPTIONS := . --exclude-dir=.vscode,libnoise,stb --fullpath --not-match-f='^.\/(src\/(gtk_)?resources\.cpp|include\/resources\.h)$$'
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)
RESGEN       := ./resgen

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

$(OUTPUT): $(OBJECTS) include/resources.h
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