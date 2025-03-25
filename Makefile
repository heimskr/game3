ifeq ($(CUSTOM_BUILD),)
	ifeq ($(BUILD),debug)
		BUILDFLAGS := -g3 -Og
	else ifeq ($(BUILD),tsan)
		BUILDFLAGS := -g3 -O1 -fsanitize=thread -fno-omit-frame-pointer
		LDFLAGS    := -fsanitize=thread
	else ifeq ($(BUILD),asan)
		BUILDFLAGS := -g3 -Og -fsanitize=address -fno-omit-frame-pointer
		LDFLAGS    := -fsanitize=address
	else ifeq ($(BUILD),ubsan)
		BUILDFLAGS := -g3 -O1 -fsanitize=undefined -fno-omit-frame-pointer
		LDFLAGS    := -fsanitize=undefined
	else ifeq ($(BUILD),nonnative)
		BUILDFLAGS := -Ofast -g3 -march=x86-64-v3
		LTO        := -flto
	else ifeq ($(BUILD),o3gnative)
		BUILDFLAGS := -O3 -g3 -march=native
		LTO        := -flto
	else ifeq ($(BUILD),release)
		BUILDFLAGS := -Ofast -march=native
		LTO        := -flto
	else
		BUILDFLAGS := -g3 -Ofast -march=native
		LTO        := -flto
	endif
else
	BUILDFLAGS := $(CUSTOM_BUILD)
	LTO        := $(CUSTOM_LTO)
endif

LDFLAGS += -lSQLiteCpp

ifeq ($(shell uname -s), Darwin)
	LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit -framework GLUT
else
	LDFLAGS += -lGL
endif

TRIPLET      ?= x64-linux
OUTPUT       := game3
COMPILER     ?= g++
DEBUGGER     ?= gdb
CPPFLAGS     += -Wall -Wextra $(BUILDFLAGS) -std=c++23 -Iinclude -Isubprojects/chemskr/include -Ibuilddir -Ibuilddir/subprojects/chemskr -Idiscord $(LTO) $(PROFILING)
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)
LDFLAGS      := $(LDFLAGS) $(LIBS) -pthread $(LTO) $(PROFILING)
SOURCES      := $(shell find -L src -name \*.cpp)
OBJECTS      := $(SOURCES:.cpp=.o)
CLOC_OPTIONS := . --exclude-dir=voronoi,pvs-report,discord,subprojects,*build*,.codechecker,_build,po,vscode,stb,eigen,json,data,.github,.idea,vcpkg_installed,build,builddir,.flatpak-builder,libnoise --fullpath --not-match-f='^\.\/((src\/(gtk_)?resources\.cpp|include\/resources\.h|analysis\.txt|include\/lib\/.*|.*\.(json|txt|md|xml))|(chemskr\/src\/chemskr/(NuclideMasses|yylex|yyparse)\.cpp|chemskr\/(include|src)\/chemskr\/yyparse\.h))$$'

.PHONY: all clean flags test

all:
	@ cd builddir && ninja

old_all: $(NOISE_OBJ) $(OUTPUT)

vcpkg_install:
	vcpkg install

all_install: vcpkg all

vcpkg_zip: all_install zip

macos_setup:
	- vcpkg install
	PKG_CONFIG_PATH=vcpkg_installed/arm64-osx/lib/pkgconfig meson setup -Dbuildtype=debugoptimized -Ddiscord_rich_presence=false -Dvcpkg_triplet=arm64-osx builddir .

macos_reconf:
	PKG_CONFIG_PATH=vcpkg_installed/arm64-osx/lib/pkgconfig meson setup -Dbuildtype=debugoptimized -Ddiscord_rich_presence=false -Dvcpkg_triplet=arm64-osx --reconfigure builddir .

macos_setup_intel:
	- vcpkg install
	PKG_CONFIG_PATH=vcpkg_installed/x64-osx/lib/pkgconfig meson setup -Ddiscord_rich_presence=false -Dvcpkg_triplet=x64-osx builddir .

macos_reconf_intel:
	PKG_CONFIG_PATH=vcpkg_installed/x64-osx/lib/pkgconfig meson setup -Ddiscord_rich_presence=false -Dvcpkg_triplet=x64-osx --reconfigure builddir .

flat:
	flatpak-builder --force-clean --arch=x86_64 --ccache --state-dir .flatpak-builder staging gay.heimskr.Game3.json
	- flatpak build-finish --command game3 --share=ipc --socket=x11 --socket=wayland --socket=fallback-x11 --share=network --socket=session-bus --device=dri staging
	flatpak build-bundle --arch=x86_64 /var/lib/flatpak/repo gay.heimskr.Game3.flatpak gay.heimskr.Game3 master

$(NOISE_OBJ):
	cd libnoise && cmake . && make

flags:
	@ echo "COMPILER: $(COMPILER)"
	@ echo
	@ echo "CPPFLAGS: $(CPPFLAGS)"
	@ echo
	@ echo "LDFLAGS:  $(LDFLAGS)"

%.o: %.cpp
	@ printf "\e[2m[\e[22;32mc++\e[39;2m]\e[22m $< \e[2m$(strip $(BUILDFLAGS) $(LTO))\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

$(OUTPUT): $(OBJECTS) chemskr/libchemskr.a $(NOISE_OBJ)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@ \e[2m$(LTO)\e[22m\n"
	@ $(COMPILER) $^ -o $@ $(LDFLAGS)

pvs:
	STAMP=$
	mkdir -p pvs-report/
	pvs-studio-analyzer trace -- meson compile -C builddir
	pvs-studio-analyzer analyze \
		-a "64;GA;OP;CS" \
		-e subprojects \
		-j "$(shell nproc)" \
		-o pvs-analysis.log
	plog-converter \
		-p game3 \
		-a "64;GA;OP;CS" \
		-t fullhtml \
		-o pvs-report/$$(date +'%Y-%m-%d_T%H%M%S') \
		-d "V104,V1042" \
		pvs-analysis.log\

repvs:
	pvs-studio-analyzer analyze \
		-a "64;GA;OP;CS" \
		-e subprojects \
		-j "$(shell nproc)" \
		-o pvs-analysis.log
	plog-converter \
		-p game3 \
		-a "64;GA;OP;CS" \
		-t fullhtml \
		-o pvs-report/$$(date +'%Y-%m-%d_T%H%M%S') \
		-d "V104,V1042" \
		pvs-analysis.log\

chemskr/libchemskr.a:
	make -C chemskr libchemskr.a

test: $(OUTPUT)
	./$<

servertest: $(OUTPUT)
	./$< -s

flasktest: $(OUTPUT)
	./$< 0 0.0 -0.2 > flask.png

tsanservertest: $(OUTPUT)
	TSAN_OPTIONS="suppressions=tsan_suppressions.txt" ./$< -s

tsanclienttest: $(OUTPUT)
	TSAN_OPTIONS="suppressions=tsan_suppressions.txt" ./$<

tsandebugtest: $(OUTPUT)
	TSAN_OPTIONS="suppressions=tsan_suppressions.txt" $(DEBUGGER) ./$<

%.tidy: %.cpp
	@ printf "\e[2m[\e[22;36mtidy\e[39;2m]\e[22m $<\n"
	@ clang-tidy $< -- $(CPPFLAGS) $(INCLUDES)

reverse = $(if $(wordlist 2,2,$(1)),$(call reverse,$(wordlist 2,$(words $(1)),$(1))) $(firstword $(1)),$(1))

tidy: $(foreach source,$(call reverse,$(SOURCES)),$(source:.cpp=.tidy))

clean:
	@ rm -f $(shell find -L src -name \*.o) $(OUTPUT) src/gtk_resources.cpp

count:
	cloc $(CLOC_OPTIONS)

countbf:
	cloc --by-file $(CLOC_OPTIONS)

analyze:
	gprof ./game3 gmon.out > analysis.txt

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -Y -- $(COMPILER) $(CPPFLAGS) -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)

winzip: game3.exe
	strip game3.exe
	rm -f game3.zip
	mkdir -p game3_zip/Game3
	cd game3_zip && \
	cp -r ../resources Game3/resources && \
	cp -r ../gamedata Game3/gamedata && \
	cp ../game3.exe Game3/ && \
	cp ../*.dll Game3/ && \
	zip -r ../game3.zip Game3

winja:
	ninja -C builddir
	cp builddir/src/game3.exe .

maczip:
	cp builddir/src/game3 ./game3
	strip game3
	rm -f game3.zip
	mkdir -p game3_zip/Game3
	cd game3_zip && \
	cp -r ../resources Game3/resources && \
	cp -r ../gamedata Game3/gamedata && \
	cp ../game3 Game3/ && \
	zip -r ../game3.zip Game3
