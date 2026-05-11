CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
LIBS    = -lSDL2 -lm
TARGET  = bin/raycaster

SRCS    = src/main.c src/cell.c src/player.c src/portal.c src/render.c
OBJS    = $(patsubst src/%.c, obj/%.o, $(SRCS))

$(TARGET): $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

bin obj:
	mkdir -p $@

clean:
	rm -rf obj bin

.PHONY: clean