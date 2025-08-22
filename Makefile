CC = gcc
CFLAGS = -Wall -Iinclude -O2 -g
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf 
SOURCES = src/main.c src/utils.c src/pathfinding.c src/game.c src/command.c src/render.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = game

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
