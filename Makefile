CC = cc
LIBS = $(shell pkg-config --cflags --libs raylib) -lm
CFLAGS = -O2 -Wall -Wpedantic -march=native -flto=auto $(LIBS)
OBJ = brickout.o

brickout: mksettings $(OBJ)
	$(CC) $(CFLAGS) -o brickout $(OBJ)

mksettings:
	test -f settings.h || make defaults

brickout.o: settings.h

tarball:
	mkdir brickout
	cp LICENSE.md README.md brickout.c settings.h brickout/
	tar czvf brickout.tar.gz brickout
	rm -rf brickout

defaults:
	rm -f settings.h
	cp settings.def.h settings.h

clean:
	rm -rf brickout brickout.tar.gz brickout $(OBJ)

cleanall: clean defaults

.PHONY: clean cleanall
