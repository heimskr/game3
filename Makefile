LTO     :=
LDFLAGS :=

ifeq ($(CUSTOM_BUILD),)
	ifeq ($(BUILD),debug)
		BUILDFLAGS := -g -O0
	else ifeq ($(BUILD),tsan)
		BUILDFLAGS := -g -O1 -fsanitize=thread
		LDFLAGS    := -fsanitize=thread
	else ifeq ($(BUILD),asan)
		BUILDFLAGS := -g -O0 -fsanitize=address -fno-omit-frame-pointer
		LDFLAGS    := -fsanitize=address
	else ifeq ($(BUILD),nonnative)
		BUILDFLAGS := -Ofast -march=x86-64-v3
		LTO        := -flto
	else ifeq ($(BUILD),o3gnative)
		BUILDFLAGS := -O3 -g -march=native
		LTO        := -flto
	else
		BUILDFLAGS := -Ofast -march=native
		LTO        := -flto
	endif
else
	BUILDFLAGS := $(CUSTOM_BUILD)
	LTO        := $(CUSTOM_LTO)
endif

ifeq ($(shell uname -s), Darwin)
	LDFLAGS := $(LDFLAGS) -framework Cocoa -framework OpenGL -framework IOKit
else
	LDFLAGS := $(LDFLAGS) -lGL
endif

DEPS         := glm glfw3 libzstd gtk4 gtkmm-4.0 glu libevent_openssl openssl libevent_pthreads
OUTPUT       := game3
COMPILER     ?= clang++
CPPFLAGS     := -Wall -Wextra $(BUILDFLAGS) -std=c++20 -Iinclude -Ijson/include -Ieigen -Istb -Ilibnoise/src $(LTO) $(PROFILING)
ZIG          ?= zig
# --main-pkg-path is needed as otherwise it wouldn't let you embed any file outside of src/
ZIGFLAGS     := -O ReleaseSmall --main-pkg-path .
ifeq ($(GITHUB),true)
	INCLUDES     := $(shell PKG_CONFIG_PATH=.github-deps/prefix/lib/pkgconfig pkg-config --cflags $(DEPS))
	LIBS         := $(shell PKG_CONFIG_PATH=.github-deps/prefix/lib/pkgconfig pkg-config --libs   $(DEPS))
	ZIG          := .zig/zig
	COMPILER     := clang++-14
else
	INCLUDES     := $(shell pkg-config --cflags $(DEPS))
	LIBS         := $(shell pkg-config --libs   $(DEPS))
endif
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread $(LTO) $(PROFILING)
SOURCES      := $(shell find src -name \*.cpp) src/gtk_resources.cpp
OBJECTS      := $(SOURCES:.cpp=.o) src/resources.o
RESXML       := $(OUTPUT).gresource.xml
CLOC_OPTIONS := . --exclude-dir=.vscode,libnoise,stb,eigen,json,data,.github --fullpath --not-match-f='^.\/(src\/(gtk_)?resources\.cpp|include\/resources\.h)$$'
RESGEN       := ./resgen
NOISE_OBJ    := libnoise/src/libnoise.a

.PHONY: all clean flags test

all: $(NOISE_OBJ) $(OUTPUT)

$(NOISE_OBJ):
	cd libnoise && cmake . && make

flags:
	@ echo "COMPILER: $(COMPILER)"
	@ echo
	@ echo "CPPFLAGS: $(CPPFLAGS)"
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
ifeq ($(GITHUB),true)
	strip $@
endif

test: $(OUTPUT)
	./$(OUTPUT)

servertest: $(OUTPUT)
	./$(OUTPUT) -s

clean:
	@ rm -f $(shell find src -name \*.o) $(OUTPUT) src/gtk_resources.cpp include/resources.h $(RESGEN) $(RESGEN).o

count:
	cloc $(CLOC_OPTIONS)

countbf:
	cloc --by-file $(CLOC_OPTIONS)

analyze:
	gprof ./game3 gmon.out > analysis.txt

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend: $(RESGEN) include/resources.h
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -Y -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	$(RESGEN) -d >> $(DEPFILE)
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)

zip: $(OUTPUT)
	rm -f game3.zip
	mkdir Game3
	cp -r resources Game3/resources
	cp -r data Game3/data
	cp $(OUTPUT) Game3/$(OUTPUT)
	zip -r game3.zip Game3
	rm -r Game3
