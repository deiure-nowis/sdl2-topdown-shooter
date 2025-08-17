#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include "types.h"

void init_console(Console* console, SDL_Renderer* renderer, TTF_Font* font);
void handle_console_input(Console* console, Player* player, World* world, SDL_Event* event);
void execute_command(Console* console, Player* player, World* world, const char* command);
void render_console(SDL_Renderer* renderer, Console* console, TTF_Font* font);
void free_console(Console* console);

#endif
