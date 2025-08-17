# SDL2 Top-Down Shooter Game

A simple top-down shooter built with SDL2 in C. Features pathfinding, FOV, enemy AI, and more.

## Building
- Install SDL2, SDL2_image, SDL2_ttf via your package manager (e.g., `brew install sdl2 sdl2_image sdl2_ttf` on macOS).
- Compile: `gcc -o game main.c game.c render.c utils.c pathfinding.c command.c -lSDL2 -lSDL2_image -lSDL2_ttf -lm`
- Run: `./game`

## License
MIT License (see LICENSE file).
