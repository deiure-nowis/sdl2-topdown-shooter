#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "types.h"
#include "common.h"

int get_heuristic(int x1, int y1, int x2, int y2);
void find_path(Enemy* enemy, World* world, int target_x, int target_y);
void move_along_path(Enemy* enemy, World* world, int enemy_index, int last_target_x, int last_target_y);
void spawn_enemy(Enemy* enemy, World* world, Camera* camera, int flag_id);
void find_cover_point(Enemy* enemy, World* world, int player_x, int player_y, int* cover_x, int* cover_y);
bool has_line_of_sightO(float x1, float y1, float x2, float y2, World* world);

#endif
