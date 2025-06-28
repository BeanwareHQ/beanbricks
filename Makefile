CC = cc
INCLUDE = -I./3rdparty/include
LIBS = $(shell pkg-config --cflags --libs raylib) -lm

CJSON_VERSION=1.7.18

OBJ = beanbricks.o config.o 3rdparty/asv/asv.o 3rdparty/cJSON/libcjson.a
SRC = beanbricks.c config.c
HEADERS = common.h config.h theme.h

RELEASE_CFLAGS = -O2 -Wall -Wextra -pedantic -march=native -flto=auto $(INCLUDE) $(LIBS)
DEBUG_CFLAGS = -O0 -g -Wall -Wextra -pedantic -fsanitize=address $(INCLUDE) $(LIBS)
TARBALLFILES = Makefile LICENSE.md README.md 3rdparty assets $(SRC) $(HEADERS)


TARGET=debug

ifeq ($(TARGET),debug)
	CFLAGS=$(DEBUG_CFLAGS)
else
	CFLAGS=$(RELEASE_CFLAGS)
endif

beanbricks: setup $(OBJ)
	$(CC) $(CFLAGS) -o beanbricks $(OBJ)

setup: deps

dep_raylib:
	mkdir -p 3rdparty/include
	test -f 3rdparty/include/raygui.h || curl -fL -o 3rdparty/include/raygui.h https://raw.githubusercontent.com/raysan5/raygui/25c8c65a6e5f0f4d4b564a0343861898c6f2778b/src/raygui.h
	
dep_asv:
	mkdir -p 3rdparty/include
	test -d 3rdparty/asv || sh -c "curl -fL -o 3rdparty/asv.zip https://github.com/ezntek/asv/archive/refs/heads/main.zip; unzip 3rdparty/asv.zip -d 3rdparty; mv 3rdparty/asv-main 3rdparty/asv"
	test -f 3rdparty/asv/asv.o || make -C 3rdparty/asv
	cp 3rdparty/asv/*.h 3rdparty/include/

dep_cjson:
	mkdir -p 3rdparty/include
	test -d 3rdparty/cJSON || sh -c "curl -fL -o 3rdparty/cJSON.tar.gz https://github.com/DaveGamble/cJSON/archive/refs/tags/v${CJSON_VERSION}.tar.gz; tar xpf 3rdparty/cJSON.tar.gz -C 3rdparty/; mv 3rdparty/cJSON-${CJSON_VERSION} 3rdparty/cJSON; make -C 3rdparty/cJSON; cp 3rdparty/cJSON/*.h 3rdparty/include/"

deps: dep_raylib dep_asv dep_cjson
		
updatedeps:
	rm -rf 3rdparty/*
	make deps

tarball: deps
	mkdir -p beanbricks
	cp -rv $(TARBALLFILES) beanbricks/
	tar czvf beanbricks.tar.gz beanbricks
	rm -rf beanbricks

defaults:
	rm -f settings.h
	cp settings.def.h settings.h

cleandeps: 
	rm -rf 3rdparty/*

clean: cleandeps
	rm -rf beanbricks beanbricks.tar.gz beanbricks $(OBJ)

cleanall: clean defaults

.PHONY: clean cleanall
