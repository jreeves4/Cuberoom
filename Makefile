all:
	gcc cuberoom.c bricks.c -o cuberoom -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lm

clean:
	rm -f cuberoom
