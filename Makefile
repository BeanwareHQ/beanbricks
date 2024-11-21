CC = cc
LIBS = $(shell pkg-config --cflags --libs raylib) -lm
CFLAGS = -O2 -march=native -flto=auto
OBJ = brickout.o

brickout: $(OBJ)
	$(CC) $(CFLAGS) -o brickout $(OBJ) $(LIBS)

brickout.o: settings.h

tarball:
	mkdir brickout
	cp LICENSE.md README.md brickout.c settings.h brickout/
	tar czvf brickout.tar.gz brickout
	rm -rf brickout

clean:
	rm -rf brickout brickout.tar.gz brickout $(OBJ)

.PHONY: clean
