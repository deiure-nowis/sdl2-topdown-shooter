#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "common.h"
#include "command.h"

void render_minimap(SDL_Renderer* renderer, Player* player, Bullet* bullets, Camera* camera, World* world, Enemy* enemies);
void init_fov_mask(SDL_Renderer* renderer, SDL_Texture** fov_mask, int w, int h);
void render_fov(SDL_Renderer* renderer, Player* player, Camera* camera, World* world, SDL_Texture* fov_mask);
bool is_in_fov(float x, float y, Player* player, World* world, float* alpha);
void render(SDL_Renderer* renderer, Player* player, Camera* camera, World* world, Bullet* bullets, Enemy* enemies, TTF_Font* font, Console* console, GameState* game_state, Menu* menu);

#endif
