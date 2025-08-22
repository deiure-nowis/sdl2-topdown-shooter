#ifndef MENU_H
#define MENU_H

#include "common.h"
#include "types.h"

void init_menu(Menu* menu, SDL_Renderer* renderer, TTF_Font* font, GameState* game_state);
void handle_menu_input(Menu* menu, SDL_Event* event, bool* running, SDL_Window* window);
void render_menu(SDL_Renderer* renderer, Menu* menu);
void free_menu(Menu* menu);

#endif
