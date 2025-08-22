# SDL2 Top-Down Shooter Game

A simple top-down shooter built with **SDL2** in C. 
Features pathfinding, FOV, enemy AI, in-game menu, etc.

---

## Features
- Top-down shooting mechanics
- Enemy AI with pathfinding
- Field of view system
- Options menu (fullscreen, spawn toggle, etc.)
- Console mechanics

---

## Building

To build the SDL2 top-down shooter, you need `SDL2`, `SDL2_image`, and `SDL2_ttf` installed on your system. The project uses a Makefile to compile the source files.

---

### Prerequisites
Install the required libraries using your package manager:

- **macOS** (using Homebrew):
```sh
  brew install sdl2 sdl2_image sdl2_ttf
```

- **Linux** (e.g., Ubuntu/Debian):
```sh
  sudo apt update
  sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

- **Windows** (using MSYS2):
```sh
  pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_ttf
```

Ensure you have gcc and make installed. On Linux/Windows, these are typically available via build-essential (Ubuntu) or MSYS2’s toolchain. On macOS, install the Xcode Command Line Tools:
  xcode-select --install

- **Compilation**
1. Clone the repository:
```sh
  git clone https://github.com/deiure-nowis/sdl2-topdown-shooter.git
  cd sdl2-topdown-shooter
```

2. Build the project using the provided Makefile:
```sh
  make
```

This compiles src/main.c, src/utils.c, src/pathfinding.c, src/game.c, src/command.c, src/render.c, and src/menu.c with flags -Wall -O2 -g and links against -lSDL2 -lSDL2_image -lSDL2_ttf.

3. Run the game:
```sh
  ./game
```
  
-  **Cleaning**
To remove compiled objects and the executable:
```sh
  make clean
```

---

## Controls
- WASD – Move

- Mouse – Aim

- Left Click – Shoot

- ESC – Open/close menu

---

## License

