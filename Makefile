CC ?= cc
LIBS = -lm
INCLUDE = -I./3rdparty/include
TARGETS = debug release small
TARGET ?= debug

CJSON_VERSION=1.7.18

SRC = beanbricks.c config.c leaderboard.c game.c titlescreen.c settings.c title.c
OBJ = beanbricks.o config.o leaderboard.o game.o titlescreen.o settings.o title.o
3RDPARTY_OBJ = 3rdparty/asv/asv.o 3rdparty/cJSON/libcjson.a
HEADERS = common.h config.h theme.h beanbricks.h game.h leaderboard.h titlescreen.h settings.h
TARBALLFILES = Makefile LICENSE.md README.md 3rdparty assets $(SRC) $(HEADERS)

ifeq (,$(filter clean cleandeps,$(MAKECMDGOALS)))

# goodbye windowze™
ifeq ($(OS),Windows_NT)
$(error building on Windows is not supported.)
endif

# check missing commands
ifeq (,$(shell command -v pkg-config))
$(error pkg-config is not installed on your system.)
else
ifeq (,$(shell pkg-config --exists raylib && echo yes))
$(error pkg-config did not find raylib.)
endif
	LIBS += $(shell pkg-config --libs raylib)
	INCLUDE += $(shell pkg-config --cflags raylib)
endif

ifeq (,$(shell command -v unzip))
$(error tar is not installed on your system.)
endif

ifeq (,$(shell command -v curl))
$(error curl is not installed on your system.)
endif

CFLAGS = -Wall -Wextra -pedantic -pipe $(INCLUDE)
ifeq ($(TARGET),debug)
	CFLAGS += -Og -fsanitize=address -g
else ifeq ($(TARGET),release)
	CFLAGS += -O2 -march=native
else ifeq ($(TARGET),small)
	CFLAGS += -Os -march=native 
else
$(error invalid target: $(TARGET))
endif

endif

build: setup $(OBJ)
	$(CC) $(LIBS) $(CFLAGS) -o beanbricks $(OBJ) $(3RDPARTY_OBJ)

# user facing stuff
targets:
	@echo "supported targets: " $(TARGETS)

# FIXME: actually implement config loading from paths
setup: deps
	[ -f config.json ] || cp defaultconfig.json config.json

fetch_raygui:
	mkdir -p 3rdparty/include
	test -f 3rdparty/include/raygui.h || curl -fL -o 3rdparty/include/raygui.h https://raw.githubusercontent.com/raysan5/raygui/25c8c65a6e5f0f4d4b564a0343861898c6f2778b/src/raygui.h
	
fetch_asv:
	mkdir -p 3rdparty/include
	if [ ! -d 3rdparty/asv ]; then \
		curl -fL -o "3rdparty/asv.zip" "https://github.com/ezntek/asv/archive/refs/heads/main.zip"; \
		unzip 3rdparty/asv.zip -d 3rdparty; \
		mv 3rdparty/asv-main 3rdparty/asv; \
	fi

	cp 3rdparty/asv/*.h 3rdparty/include/

fetch_cjson:
	mkdir -p 3rdparty/include

	if [ ! -d 3rdparty/cJSON ]; then \
		curl -fL -o 3rdparty/cJSON.tar.gz https://github.com/DaveGamble/cJSON/archive/refs/tags/v$(CJSON_VERSION).tar.gz; \
		tar xpf 3rdparty/cJSON.tar.gz -C 3rdparty/; \
		mv 3rdparty/cJSON-$(CJSON_VERSION) 3rdparty/cJSON; \
		cp 3rdparty/cJSON/*.h 3rdparty/include/; \
	fi

fetch_deps: fetch_raygui fetch_asv fetch_cjson

# raygui needs no compilation
deps: fetch_deps
	$(MAKE) -C 3rdparty/asv
	$(MAKE) -C 3rdparty/cJSON
		
updatedeps:
	rm -rf 3rdparty/*
	$(MAKE) fetch_deps

builddeps: deps

tarball: deps
	mkdir -p beanbricks
	cp -rv $(TARBALLFILES) beanbricks/
	tar czvf beanbricks.tar.gz beanbricks
	rm -rf beanbricks

cleandeps: 
	rm -rf 3rdparty/*

clean:
	rm -rf beanbricks beanbricks.tar.gz beanbricks $(OBJ)

cleanall: clean cleandeps

.PHONY: clean cleandeps cleanall 
