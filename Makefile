CC = cc
INCLUDE = -I./3rdparty/include
LIBS = $(shell pkg-config --cflags --libs raylib) -lm

CJSON_VERSION=1.7.18

OBJ = beanbricks.o config.o 3rdparty/asv/asv.o 3rdparty/cJSON/libcjson.a

RELEASE_CFLAGS = -O2 -Wall -Wextra -pedantic -march=native -flto=auto $(INCLUDE) $(LIBS)
DEBUG_CFLAGS = -O0 -g -Wall -Wextra -pedantic -fsanitize=address $(INCLUDE) $(LIBS)
TARBALLFILES = Makefile LICENSE.md README.md beanbricks.c settings.def.h 3rdparty assets

HEADERS = settings.h

TARGET=debug

ifeq ($(TARGET),debug)
	CFLAGS=$(DEBUG_CFLAGS)
else
	CFLAGS=$(RELEASE_CFLAGS)
endif

beanbricks: setup $(OBJ)
	$(CC) $(CFLAGS) -o beanbricks $(OBJ)

setup: deps settings

settings:
	test -f settings.h || make defaults

beanbricks.o: settings.h

dep_raylib:
	mkdir -p 3rdparty/include
	test -f 3rdparty/include/raygui.h || curl -fL -o 3rdparty/include/raygui.h https://raw.githubusercontent.com/raysan5/raygui/25c8c65a6e5f0f4d4b564a0343861898c6f2778b/src/raygui.h
	
dep_asv:
	test -d 3rdparty/asv || curl -fL -o 3rdparty/asv.zip https://github.com/ezntek/asv/archive/refs/heads/main.zip && unzip 3rdparty/asv.zip -d 3rdparty && mv 3rdparty/asv-main 3rdparty/asv 
	test -f 3rdparty/asv/asv.o || make -C 3rdparty/asv
	cp 3rdparty/asv/*.h 3rdparty/include/

dep_cjson:
	test -d 3rdparty/cJSON || curl -fL -o 3rdparty/cJSON.tar.gz https://github.com/DaveGamble/cJSON/archive/refs/tags/v${CJSON_VERSION}.tar.gz && tar xpf 3rdparty/cJSON.tar.gz -C 3rdparty/ && mv 3rdparty/cJSON-${CJSON_VERSION} 3rdparty/cJSON && make -C 3rdparty/cJSON && cp 3rdparty/cJSON/*.h 3rdparty/include/

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

clean:
	rm -rf beanbricks beanbricks.tar.gz beanbricks $(OBJ)
	rm -f 3rdparty/include/*

cleanall: clean defaults

.PHONY: clean cleanall
