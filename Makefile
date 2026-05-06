CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lSDL2 -lm
 
TARGET = raycaster
SRCS = main.c cell.c
 
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)
 
clean:
	rm -f $(TARGET)
 
.PHONY: clean
 
