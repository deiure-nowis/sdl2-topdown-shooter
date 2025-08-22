#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "common.h"

void update_camera(Camera* camera, Player* player, World* world, SDL_Renderer* renderer);
void update_player_angle(Camera* camera, Player* player);
void spawn_bullet(Bullet* bullets, Entity* entity, int owner);
void update_bullets(Bullet* bullets, World* world, float dt);
void fixed_update_player(Player* player, World* world, Bullet* bullets, Camera* camera, Console* console);
void fixed_update_enemies(Player* player, World* world, Bullet* bullets, Enemy* enemies, Camera* camera, GameState* game_state);

#endif
