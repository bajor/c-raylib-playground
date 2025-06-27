CC      = clang
CFLAGS  = -std=c99 -Wall -Wextra -O2 $(shell pkg-config --cflags raylib)
LDFLAGS = $(shell pkg-config --libs raylib)

all: run

run: clean game
	./game

game: main.c level.c dots.c
	$(CC) main.c level.c dots.c -o game $(CFLAGS) $(LDFLAGS)

clean:
	rm -f game

.PHONY: clean all run
