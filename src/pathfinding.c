#include "pathfinding.h"
#include "utils.h"

int get_heuristic(int x1, int y1, int x2, int y2) {
	int dx = absi(x1 - x2);
	int dy = absi(y1 - y2);
	return 10 * (dx + dy) + (14 - 2 * 10) * (dx < dy ? dx : dy);
}

void find_path(Enemy* enemy, World* world, int target_x, int target_y) {
	if (!enemy || !world) {
		printf("Error: Null enemy or world in find_path\n");
		return;
	}
	if (!is_valid_node(target_x, target_y, world)) {
		enemy->path_length = 0;
		return;
	}

	Node open_list[MAP_SIZE * MAP_SIZE];
	Node closed_list[MAP_SIZE * MAP_SIZE];
	int open_count = 0, closed_count = 0;
	int iterations = 0;

	int start_x = (int)((enemy->x + enemy->w / 2) / TILE_SIZE);
	int start_y = (int)((enemy->y + enemy->h / 2) / TILE_SIZE);

	if (!is_valid_node(start_x, start_y, world)) {
		enemy->path_length = 0;
		return;
	}

	open_list[open_count++] = (Node){start_x, start_y, 0, get_heuristic(start_x, start_y, target_x, target_y), 0, -1, -1};
	open_list[0].f = open_list[0].g + open_list[0].h;

	while (open_count > 0 && iterations < MAX_PATHFINDING_ITERATIONS) {
		iterations++;
		int best_index = 0;
		for (int i = 1; i < open_count; i++) {
			if (open_list[i].f < open_list[best_index].f) best_index = i;
		}

		Node current = open_list[best_index];
		open_list[best_index] = open_list[--open_count];
		closed_list[closed_count++] = current;

		if (current.x == target_x && current.y == target_y) {
			enemy->path_length = 0;
			while (current.x != -1 && current.y != -1) {
				if (enemy->path_length >= MAP_SIZE * MAP_SIZE) {
					enemy->path_length = 0;
					printf("Error: Path length exceeded in find_path\n");
					return;
				}
				enemy->path[enemy->path_length++] = current.x + current.y * MAP_SIZE;
				if (current.parent_x == -1 && current.parent_y == -1) break;
				bool found_parent = false;
				for (int i = 0; i < closed_count; i++) {
					if (closed_list[i].x == current.parent_x && closed_list[i].y == current.parent_y) {
						current = closed_list[i];
						found_parent = true;
						break;
					}
				}
				if (!found_parent) {
					enemy->path_length = 0;
					printf("Error: Parent not found in find_path\n");
					return;
				}
			}

			if (enemy->path_length > 1) {
				for (int i = 1; i < enemy->path_length; i++) {
					enemy->path[i - 1] = enemy->path[i];
				}
				enemy->path_length--;
			} else if (enemy->path_length == 1) {
				enemy->path_length = 0;
			}
			return;
		}

		int directions[8][2] = {
			{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
		};
		int costs[8] = {10, 10, 10, 10, 14, 14, 14, 14};
		for (int i = 0; i < 8; i++) {
			int new_x = current.x + directions[i][0];
			int new_y = current.y + directions[i][1];
			if (!is_valid_node(new_x, new_y, world)) continue;

			if (i >= 4) {
				int ortho_x1 = current.x + directions[i][0];
				int ortho_y1 = current.y;
				int ortho_x2 = current.x;
				int ortho_y2 = current.y + directions[i][1];
				if (!is_valid_node(ortho_x1, ortho_y1, world) || !is_valid_node(ortho_x2, ortho_y2, world)) {
					continue;
				}
			}

			bool in_closed = false;
			for (int j = 0; j < closed_count; j++) {
				if (closed_list[j].x == new_x && closed_list[j].y == new_y) {
					in_closed = true;
					break;
				}
			}
			if (in_closed) continue;

			int new_g = current.g + costs[i];
			int new_h = get_heuristic(new_x, new_y, target_x, target_y);
			int new_f = new_g + new_h;

			bool in_open = false;
			int open_index = -1;
			for (int j = 0; j < open_count; j++) {
				if (open_list[j].x == new_x && open_list[j].y == new_y) {
					in_open = true;
					open_index = j;
					break;
				}
			}

			if (in_open) {
				if (new_g < open_list[open_index].g) {
					open_list[open_index].g = new_g;
					open_list[open_index].f = new_f;
					open_list[open_index].parent_x = current.x;
					open_list[open_index].parent_y = current.y;
				}
			} else {
				if (open_count < MAP_SIZE * MAP_SIZE) {
					open_list[open_count++] = (Node){new_x, new_y, new_g, new_h, new_f, current.x, current.y};
				}
			}
		}
	}
	enemy->path_length = 0;
}

void move_along_path(Enemy* enemy, World* world, int enemy_index, int last_target_x, int last_target_y) {
    if (!enemy || !world || enemy_index < 0 || enemy_index >= MAX_ENEMIES) {
        printf("Error: Invalid enemy, world, or enemy_index (%d) in move_along_path\n", enemy_index);
        return;
    }

    static float stuck_timer[MAX_ENEMIES] = {0.0f};
    static float last_x[MAX_ENEMIES] = {0.0f};
    static float last_y[MAX_ENEMIES] = {0.0f};
    static bool force_path_recalc[MAX_ENEMIES] = {false};

    // Angle update function, aligned with player demo
    void update_angle(float target_x, float target_y) {
        float angle_dx = target_x - (enemy->x + enemy->w / 2);
        float angle_dy = target_y - (enemy->y + enemy->h / 2);
        float target_angle = my_atan2f(angle_dy, angle_dx) * (180.0f / MA_PI);
        float angle_diff = target_angle - enemy->angle;

        // Normalize angle difference to [-180, 180]
        while (angle_diff > 180.0f) angle_diff -= 360.0f;
        while (angle_diff < -180.0f) angle_diff += 360.0f;

        // Stop rotation if within a small threshold to prevent jitter
        if (absf(angle_diff) < 2.0f) {
            enemy->angle = target_angle; // Snap to target to avoid micro-adjustments
            return;
        }

        // Cap rotation speed
        float max_rotation = ENEMY_ROTATION_SPEED * FIXED_DT;
        if (angle_diff > max_rotation) angle_diff = max_rotation;
        if (angle_diff < -max_rotation) angle_diff = -max_rotation;

        enemy->angle += angle_diff;
        if (enemy->angle > 180.0f) enemy->angle -= 360.0f;
        if (enemy->angle < -180.0f) enemy->angle += 360.0f;
    }

    // Handle no path
    if (enemy->path_length <= 0) {
        // Only rotate toward player in SHOOT or TAKE_COVER when stationary
        if (enemy->state == SHOOT || (enemy->state == TAKE_COVER && enemy->path_length == 0)) {
            if (last_target_x != -1 && last_target_y != -1) {
                float target_x = last_target_x * TILE_SIZE + TILE_SIZE / 2;
                float target_y = last_target_y * TILE_SIZE + TILE_SIZE / 2;
                update_angle(target_x, target_y);
            }
        }
        // In FREE state, maintain current angle (no reset to 0.0f)
        enemy->vel_x = 0.0f;
        enemy->vel_y = 0.0f;
        stuck_timer[enemy_index] = 0.0f;
        force_path_recalc[enemy_index] = false;
        return;
    }

    // Get next path node
    int next_node_index = enemy->path_length - 1;
    int next_x = (enemy->path[next_node_index] % MAP_SIZE) * TILE_SIZE + TILE_SIZE / 2;
    int next_y = (enemy->path[next_node_index] / MAP_SIZE) * TILE_SIZE + TILE_SIZE / 2;

    float dx = next_x - (enemy->x + enemy->w / 2);
    float dy = next_y - (enemy->y + enemy->h / 2);
    float dist = my_sqrt(dx * dx + dy * dy);

    // Reached the node
    if (dist < TILE_SIZE * 0.5f) {
        enemy->vel_x = 0.0f;
        enemy->vel_y = 0.0f;
        enemy->path_length--;
        if (enemy->path_length <= 0) {
            if (enemy->state == SHOOT || (enemy->state == TAKE_COVER && enemy->path_length == 0)) {
                if (last_target_x != -1 && last_target_y != -1) {
                    float target_x = last_target_x * TILE_SIZE + TILE_SIZE / 2;
                    float target_y = last_target_y * TILE_SIZE + TILE_SIZE / 2;
                    update_angle(target_x, target_y);
                }
            }
            // In FREE state, maintain current angle (no reset to 0.0f)
        }
        return;
    }
    update_angle(next_x, next_y);

    // Update velocity (continuous movement)
    if (dist > 0.0f) {
        float speed = ENEMY_SPEED * FIXED_DT;
        enemy->vel_x = (dx / dist) * speed;
        enemy->vel_y = (dy / dist) * speed;
    } else {
        enemy->vel_x = 0.0f;
        enemy->vel_y = 0.0f;
    }

    // Check if stuck
    float movement = my_sqrt((enemy->x - last_x[enemy_index]) * (enemy->x - last_x[enemy_index]) +
                            (enemy->y - last_y[enemy_index]) * (enemy->y - last_y[enemy_index]));
    if (movement < 1.0f && enemy->path_length > 0) {
        stuck_timer[enemy_index] += FIXED_DT;
        if (stuck_timer[enemy_index] >= STUCK_THRESHOLD) {
            printf("Enemy %d stuck at (%.2f, %.2f) tile (%d, %d), targeting (%d, %d), path_length=%d\n",
                enemy_index, enemy->x, enemy->y,
                (int)(enemy->x / TILE_SIZE), (int)(enemy->y / TILE_SIZE),
                next_x / TILE_SIZE, next_y / TILE_SIZE, enemy->path_length);
            force_path_recalc[enemy_index] = true;
            stuck_timer[enemy_index] = 0.0f;
        }
    } else {
        stuck_timer[enemy_index] = 0.0f;
        force_path_recalc[enemy_index] = false;
    }
    last_x[enemy_index] = enemy->x;
    last_y[enemy_index] = enemy->y;

    enemy->force_path_recalc = force_path_recalc[enemy_index];
}

void spawn_enemy(Enemy* enemy, World* world, Camera* camera, int flag_id) {
    if (!enemy || !world || !camera) {
        printf("Error: Null enemy, world, or camera in spawn_enemy\n");
        return;
    }

    enemy->active = true;
    enemy->hp = ENEMY_HP;
    enemy->w = 48;
    enemy->h = 48;
    enemy->path_length = 0;
    enemy->path_timer = 0.0f;
    enemy->respawn_timer = 0.0f;
    enemy->vel_x = 0.0f;
    enemy->vel_y = 0.0f;
    enemy->angle = (float)(rand() % 360);
    enemy->state = FREE;
    enemy->shoot_timer = 0.0f;
    enemy->decision_timer = 0.0f;
    enemy->in_cover = false;
    enemy->flag_id = flag_id; // Associate with flag

    float spawn_center_x, spawn_center_y;
    if (flag_id >= 0 && flag_id < world->flag_count && world->flags[flag_id].active) {
        spawn_center_x = world->flags[flag_id].x + world->flags[flag_id].w / 2;
        spawn_center_y = world->flags[flag_id].y + world->flags[flag_id].h / 2;
    } else {
        spawn_center_x = camera->x + CAMERA_W / 2;
        spawn_center_y = camera->y + CAMERA_H / 2;
    }

    bool valid_spawn = false;
    int attempts = 0;
    const int max_attempts = 100;
    float spawn_x, spawn_y;

    while (!valid_spawn && attempts < max_attempts) {
        attempts++;
        float angle = (float)(rand() % 360) * (MA_PI / 180.0f);
        float radius = 100.0f + (rand() % 100); // Spawn within 100-200 pixels
        spawn_x = spawn_center_x + my_cosf(angle) * radius;
        spawn_y = spawn_center_y + my_sinf(angle) * radius;

        int tile_x = (int)(spawn_x / TILE_SIZE);
        int tile_y = (int)(spawn_y / TILE_SIZE);
        if (!is_valid_node(tile_x, tile_y, world)) continue;

        float dx = spawn_x - (camera->x + CAMERA_W / 2);
        float dy = spawn_y - (camera->y + CAMERA_H / 2);
        float distance = my_sqrt(dx * dx + dy * dy);
        if (distance < PATHFINDING_RANGE) continue;

        valid_spawn = true;
    }

    if (!valid_spawn) {
        spawn_x = spawn_center_x;
        spawn_y = spawn_center_y;
    }

    enemy->x = spawn_x - enemy->w / 2;
    enemy->y = spawn_y - enemy->h / 2;
}

void find_cover_point(Enemy* enemy, World* world, int player_x, int player_y, int* cover_x, int* cover_y) {
	if (!enemy || !world || !cover_x || !cover_y) {
		*cover_x = -1;
		*cover_y = -1;
		return;
	}

	//int enemy_tile_x = (int)((enemy->x + enemy->w / 2) / TILE_SIZE);
	//int enemy_tile_y = (int)((enemy->y + enemy->h / 2) / TILE_SIZE);
	int best_x = -1, best_y = -1;
	float best_score = FLT_MAX;
	int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}}; // Orthogonal directions

	// Search for small walls within radius
	for (int y = 0; y < MAP_SIZE; y++) {
		for (int x = 0; x < MAP_SIZE; x++) {
			if (world->map[y][x] != WALL_SMALL) continue;

			float wall_pixel_x = x * TILE_SIZE + TILE_SIZE / 2;
			float wall_pixel_y = y * TILE_SIZE + TILE_SIZE / 2;
			float dx = wall_pixel_x - (enemy->x + enemy->w / 2);
			float dy = wall_pixel_y - (enemy->y + enemy->h / 2);
			float dist_to_wall = my_sqrt(dx * dx + dy * dy);
			if (dist_to_wall > MAX_COVER_DISTANCE) continue;

			// Check adjacent tiles for valid cover positions
			for (int i = 0; i < 4; i++) {
				int cx = x + directions[i][0];
				int cy = y + directions[i][1];
				if (!is_valid_node(cx, cy, world)) continue;

				float cover_pixel_x = cx * TILE_SIZE + TILE_SIZE / 2;
				float cover_pixel_y = cy * TILE_SIZE + TILE_SIZE / 2;
				float dist_to_player = my_sqrt(
					(cover_pixel_x - (player_x * TILE_SIZE + TILE_SIZE / 2)) *
					(cover_pixel_x - (player_x * TILE_SIZE + TILE_SIZE / 2)) +
					(cover_pixel_y - (player_y * TILE_SIZE + TILE_SIZE / 2)) *
					(cover_pixel_y - (player_y * TILE_SIZE + TILE_SIZE / 2))
				);

				// Check if within shooting range
				if (dist_to_player < SHOOTING_RANGE * 0.8f || dist_to_player > SHOOTING_RANGE * 1.2f) continue;

				// Check line-of-sight (no BULLETPROOF or OPAQUE walls)
				if (!has_line_of_sight(cover_pixel_x, cover_pixel_y,
                                       player_x * TILE_SIZE + TILE_SIZE / 2,
                                       player_y * TILE_SIZE + TILE_SIZE / 2,
                                       world, true, true)) {
					continue;
				}

				// Check if the small wall is between the cover point and player
				bool wall_between = false;
				float wall_dx = wall_pixel_x - cover_pixel_x;
				float wall_dy = wall_pixel_y - cover_pixel_y;
				float player_dx = (player_x * TILE_SIZE + TILE_SIZE / 2) - cover_pixel_x;
				float player_dy = (player_y * TILE_SIZE + TILE_SIZE / 2) - cover_pixel_y;

				// Check if wall is roughly in the direction of the player
				float dot_product = wall_dx * player_dx + wall_dy * player_dy;
				float wall_dist = my_sqrt(wall_dx * wall_dx + wall_dy * wall_dy);
				float player_dist = my_sqrt(player_dx * player_dx + player_dy * player_dy);
				if (dot_product > 0 && wall_dist < player_dist) {
					// Wall is closer to cover point than player and in the same direction
					wall_between = true;
				}

				// Score the cover point
				float score = dist_to_wall + dist_to_player * 0.5f;
				if (wall_between) {
					score *= 0.5f; // Prioritize positions with wall between
				}

				if (score < best_score) {
					best_score = score;
					best_x = cx;
					best_y = cy;
				}
			}
		}
	}

	*cover_x = best_x;
	*cover_y = best_y;
}
