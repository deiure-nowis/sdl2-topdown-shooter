#include "game.h"
#include "render.h"
#include "utils.h"
#include "pathfinding.h"

void update_camera(Camera* camera, Player* player, World* world, SDL_Renderer* renderer) {
    if (!renderer) {
        printf("Error: Null renderer in update_camera\n");
        camera->w = CAMERA_W; // Fallback to default dimensions
        camera->h = CAMERA_H;
    } else {
        int renderer_w, renderer_h;
        if (SDL_GetRendererOutputSize(renderer, &renderer_w, &renderer_h) != 0) {
            printf("Error: SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
            camera->w = CAMERA_W; // Fallback
            camera->h = CAMERA_H;
        } else {
            camera->w = renderer_w;
            camera->h = renderer_h;
        }
    }

    // Center camera on player's exact center
    float player_center_x = player->x + player->w / 2.0f;
    float player_center_y = player->y + player->h / 2.0f;
    camera->x = player_center_x - (camera->w / 2.0f);
    camera->y = player_center_y - (camera->h / 2.0f);

    // Clamp camera to world bounds
    if (camera->x < 0) camera->x = 0;
    if (camera->y < 0) camera->y = 0;
    if (camera->x + camera->w > world->w) camera->x = world->w - camera->w;
    if (camera->y + camera->h > world->h) camera->y = world->h - camera->h;
}

void update_player_angle(Camera* camera, Player* player){
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    float world_mouse_x = mouse_x + camera->x;
    float world_mouse_y = mouse_y + camera->y;
    float gun_pivot_x = player->x + player->w / 2;
    float gun_pivot_y = player->y + player->h / 2;
    float dx = world_mouse_x - gun_pivot_x;
    float dy = world_mouse_y - gun_pivot_y;
    float target_angle = my_atan2f(dy, dx) * (180.0f / MA_PI);

	float angle_diff = target_angle - player->angle;
	while(angle_diff > 180.0f) angle_diff -= 360.0f;
	while(angle_diff < -180.0f) angle_diff += 360.0f;
	if(absf(angle_diff) < MAX_PLAYER_ROTATION){
		player->angle = target_angle;
	}else{
		if(angle_diff > MAX_PLAYER_ROTATION) angle_diff = MAX_PLAYER_ROTATION;
		if(angle_diff < MAX_PLAYER_ROTATION) angle_diff = -MAX_PLAYER_ROTATION;
		player->angle += angle_diff;
		if(player->angle > 180.0f) player->angle -= 360.0f;
		if(player->angle < -180.0f) player->angle += 360.0f;
	}
}

void spawn_bullet(Bullet* bullets, Entity* entity, int owner) {
    float angle_rad = entity->angle * (MA_PI / 180.0f);
    float cos_a = my_cosf(angle_rad);
    float sin_a = my_sinf(angle_rad);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = entity->x + entity->w / 2 + cos_a * (entity->w / 2);
            bullets[i].y = entity->y + entity->h / 2 + sin_a * (entity->h / 2);
            bullets[i].vel_x = cos_a * BULLET_SPEED;
            bullets[i].vel_y = sin_a * BULLET_SPEED;
            bullets[i].lifetime = BULLET_LIFETIME;
            bullets[i].owner = owner;
            bullets[i].active = true;
            break;
        }
    }
}

void update_bullets(Bullet* bullets, World* world, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            float next_x = bullets[i].x + bullets[i].vel_x * dt;
            float next_y = bullets[i].y + bullets[i].vel_y * dt;
            bool collide = false;
            for (int j = 0; j < world->wall_count; j++) {
                if ((world->walls[j].type == WALL_BULLETPROOF || world->walls[j].type == WALL_OPAQUE) &&
                    check_collision(next_x, next_y, 5, 5, world->walls[j].x,
                                    world->walls[j].y, world->walls[j].w,
                                    world->walls[j].h)) {
                    bullets[i].active = false;
                    collide = true;
                    break;
                }
            }
            if (!collide) {
                bullets[i].x = next_x;
                bullets[i].y = next_y;
                bullets[i].lifetime -= dt;
                if (bullets[i].lifetime <= 0 || bullets[i].x < 0 || bullets[i].x > world->w ||
                    bullets[i].y < 0 || bullets[i].y > world->h) {
                    bullets[i].active = false;
                }
            }
        }
    }
}

void fixed_update_player(Player* player, World* world, Bullet* bullets, Camera* camera, Console* console) {
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    float speed = 300.0f;
    float friction = 0.9f;
    float vel_input_x = 0.0f, vel_input_y = 0.0f;

    if (!console->active) {
        if (keyboard_state[SDL_SCANCODE_W]) vel_input_y -= speed * FIXED_DT;
        if (keyboard_state[SDL_SCANCODE_S]) vel_input_y += speed * FIXED_DT;
        if (keyboard_state[SDL_SCANCODE_A]) vel_input_x -= speed * FIXED_DT;
        if (keyboard_state[SDL_SCANCODE_D]) vel_input_x += speed * FIXED_DT;
        update_player_angle(camera, player);
    }

    player->vel_x += vel_input_x;
    player->vel_y += vel_input_y;
    player->vel_x *= friction;
    player->vel_y *= friction;

    float length = my_sqrt(player->vel_x * player->vel_x + player->vel_y * player->vel_y);
    if (length > speed * FIXED_DT && length > 0.0f) {
        player->vel_x = (player->vel_x / length) * speed * FIXED_DT;
        player->vel_y = (player->vel_y / length) * speed * FIXED_DT;
    }

    float next_x = player->x + player->vel_x;
    float next_y = player->y + player->vel_y;
    bool collide_x = false, collide_y = false;

    for (int i = 0; i < world->wall_count; i++) {
        if (world->walls[i].type != WALL_NONE &&
            check_collision(next_x + 10, player->y + 10, player->w - 20, player->h - 20,
                            world->walls[i].x, world->walls[i].y,
                            world->walls[i].w, world->walls[i].h)) {
            collide_x = true;
        }
        if (world->walls[i].type != WALL_NONE &&
            check_collision(player->x + 10, next_y + 10, player->w - 20, player->h - 20,
                            world->walls[i].x, world->walls[i].y,
                            world->walls[i].w, world->walls[i].h)) {
            collide_y = true;
        }
    }

    if (!collide_x) player->x += player->vel_x;
    if (!collide_y) player->y += player->vel_y;

    if (player->x < 0) player->x = 0;
    if (player->x + player->w > world->w) player->x = world->w - player->w;
    if (player->y < 0) player->y = 0;
    if (player->y + player->h > world->h) player->y = world->h - player->h;

    update_bullets(bullets, world, FIXED_DT);
}

void fixed_update_enemies(Player* player, World* world, Bullet* bullets, Enemy* enemies, Camera* camera, GameState* game_state) {
    static float target_x_history[MAX_ENEMIES][3] = {0};
    static float target_y_history[MAX_ENEMIES][3] = {0};
    static int history_index[MAX_ENEMIES] = {0};
    static int last_target_x[MAX_ENEMIES] = { -1 };
    static int last_target_y[MAX_ENEMIES] = { -1 };
    static float search_timer[MAX_ENEMIES] = {0.0f};
    static float look_around_timer[MAX_ENEMIES] = {0.0f};
    static float base_angle[MAX_ENEMIES] = {0.0f};

    // Flag & spawn logic
    if (game_state->spawn_enabled) {
        for (int i = 0; i < world->flag_count; i++) {
            if (!world->flags[i].active) continue;
            world->flags[i].spawn_timer -= FIXED_DT;
            if (world->flags[i].spawn_timer <= 0.0f &&
                world->flags[i].enemies_spawned < world->flags[i].enemy_count) {
                for (int j = 0; j < MAX_ENEMIES &&
                                world->flags[i].enemies_spawned < world->flags[i].enemy_count; j++) {
                    if (!enemies[j].active) {
                        spawn_enemy(&enemies[j], world, camera, i);
                        world->flags[i].enemies_spawned++;
                    }
                }
                world->flags[i].spawn_timer = ENEMY_RESPAWN_DELAY;
            }
        }
    }

    // Enemy update loop
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            // handle respawn countdown
            if (game_state->spawn_enabled && enemies[i].respawn_timer > 0.0f) {
                enemies[i].respawn_timer -= FIXED_DT;
                if (enemies[i].respawn_timer <= 0.0f &&
                    enemies[i].flag_id >= 0 &&
                    enemies[i].flag_id < world->flag_count) {
                    spawn_enemy(&enemies[i], world, camera, enemies[i].flag_id);
                }
            }
            continue;
        }

        // AI perception
        float dx = (player->x + player->w / 2) - (enemies[i].x + enemies[i].w / 2);
        float dy = (player->y + player->h / 2) - (enemies[i].y + enemies[i].h / 2);
        float distance = my_sqrt(dx * dx + dy * dy);

        float angle_to_player = my_atan2f(dy, dx) * (180.0f / MA_PI);
        float angle_diff = angle_to_player - enemies[i].angle;
        while (angle_diff > 180.0f) angle_diff -= 360.0f;
        while (angle_diff < -180.0f) angle_diff += 360.0f;
        bool in_fov = absf(angle_diff) <= ENEMY_HALF_ANGLE;

        // smooth target tracking
        float raw_target_x = (player->x + player->w / 2) / TILE_SIZE;
        float raw_target_y = (player->y + player->h / 2) / TILE_SIZE;
        target_x_history[i][history_index[i]] = raw_target_x;
        target_y_history[i][history_index[i]] = raw_target_y;
        history_index[i] = (history_index[i] + 1) % 3;

        float avg_target_x = 0.0f, avg_target_y = 0.0f;
        for (int j = 0; j < 3; j++) {
            avg_target_x += target_x_history[i][j];
            avg_target_y += target_y_history[i][j];
        }
        avg_target_x /= 3.0f;
        avg_target_y /= 3.0f;
        int target_x = (int)avg_target_x;
        int target_y = (int)avg_target_y;

        // timers
        enemies[i].decision_timer -= FIXED_DT;
        enemies[i].shoot_timer -= FIXED_DT;
        if (enemies[i].state == SEARCHING) {
            search_timer[i] -= FIXED_DT;
        }

        // Decision making
        if (enemies[i].decision_timer <= 0.0f || enemies[i].force_path_recalc) {
            enemies[i].decision_timer = DECISION_INTERVAL;

            bool has_los = has_line_of_sight(
                enemies[i].x + enemies[i].w / 2.0f,
                enemies[i].y + enemies[i].h / 2.0f,
                player->x + player->w / 2.0f,
                player->y + player->h / 2.0f,
                world, false, true
            );

			Entity enemy_entity = {enemies[i].x, enemies[i].y,
								   enemies[i].w, enemies[i].h,
								   enemies[i].angle,
								   {0},
								   enemies[i].path_length};

            if (distance > PATHFINDING_RANGE || !in_fov || !has_los) {
                // lost the player
                if (enemies[i].state == CHASE ||
                    enemies[i].state == SHOOT ||
                    enemies[i].state == TAKE_COVER) {
                    enemies[i].state = SEARCHING;
                    search_timer[i] = 5.0f;
                    if (last_target_x[i] != -1 && last_target_y[i] != -1) {
                        float target_world_x = last_target_x[i] * TILE_SIZE + TILE_SIZE / 2;
                        float target_world_y = last_target_y[i] * TILE_SIZE + TILE_SIZE / 2;
                        float dx_target = target_world_x - (enemies[i].x + enemies[i].w / 2);
                        float dy_target = target_world_y - (enemies[i].y + enemies[i].h / 2);
                        base_angle[i] = my_atan2f(dy_target, dx_target) * (180.0f / MA_PI);

                        find_path(&enemy_entity, world, last_target_x[i], last_target_y[i]);
                        memcpy(enemies[i].path, enemy_entity.path, sizeof(enemies[i].path));
                        enemies[i].path_length = enemy_entity.path_length;
                    } else {
                        enemies[i].path_length = 0;
                    }
                    enemies[i].in_cover = false;
                } else if (enemies[i].state == SEARCHING && search_timer[i] <= 0.0f) {
                    enemies[i].state = FREE;
                    enemies[i].path_length = 0;
                    enemies[i].vel_x = 0.0f;
                    enemies[i].vel_y = 0.0f;
                    enemies[i].in_cover = false;
                    last_target_x[i] = -1;
                    last_target_y[i] = -1;
                } else if (enemies[i].state == FREE) {
                    if (enemies[i].path_length == 0) {
                        if ((rand() % 5) == 0) {
                            int walk_range_min = 3;
                            int walk_range_max = 10;
                            int enemy_tile_x = (int)((enemies[i].x + enemies[i].w / 2) / TILE_SIZE);
                            int enemy_tile_y = (int)((enemies[i].y + enemies[i].h / 2) / TILE_SIZE);
                            int attempts = 0;
                            int max_attempts = 10;
                            int walk_x, walk_y;
                            bool valid_walk = false;

                            while (!valid_walk && attempts < max_attempts) {
                                int walk_distance = (rand() % (walk_range_max - walk_range_min + 1)) + walk_range_min;
                                int angle = rand() % 360;
                                int dx = (int)(my_cosf(angle * MA_PI / 180.0f) * walk_distance);
                                int dy = (int)(my_sinf(angle * MA_PI / 180.0f) * walk_distance);
                                walk_x = enemy_tile_x + dx;
                                walk_y = enemy_tile_y + dy;

                                if (is_valid_node(walk_x, walk_y, world)) {
                                    valid_walk = true;
                                }
                                attempts++;
                            }

                            if (valid_walk) {
                                find_path(&enemy_entity, world, walk_x, walk_y);
                                memcpy(enemies[i].path, enemy_entity.path, sizeof(enemies[i].path));
                                enemies[i].path_length = enemy_entity.path_length;
                                last_target_x[i] = walk_x;
                                last_target_y[i] = walk_y;
                            }
                        }
                    }
                }
            } else {
                // player detected
                if (enemies[i].state == FREE || enemies[i].state == SEARCHING) {
                    enemies[i].path_length = 0;
                }

                int cover_x, cover_y;
                find_cover_point(&enemies[i], world, target_x, target_y, &cover_x, &cover_y);

                if (distance < SHOOTING_RANGE) {
                    if (cover_x != -1 && cover_y != -1) {
                        enemies[i].state = TAKE_COVER;
                        find_path(&enemy_entity, world, cover_x, cover_y);
                        memcpy(enemies[i].path, enemy_entity.path, sizeof(enemies[i].path));
                        enemies[i].path_length = enemy_entity.path_length;
                        enemies[i].in_cover = true;
                    } else {
                        enemies[i].state = SHOOT;
                        enemies[i].path_length = 0;
                        enemies[i].in_cover = false;
                    }
                } else {
                    enemies[i].state = CHASE;
                    find_path(&enemy_entity, world, target_x, target_y);
                    memcpy(enemies[i].path, enemy_entity.path, sizeof(enemies[i].path));
                    enemies[i].path_length = enemy_entity.path_length;
                    enemies[i].in_cover = false;
                }
                last_target_x[i] = target_x;
                last_target_y[i] = target_y;
            }
            enemies[i].path_timer = PATHFINDING_INTERVAL;
            enemies[i].force_path_recalc = false;
        }

        // Searching animation
        if (enemies[i].state == SEARCHING && enemies[i].path_length == 0) {
            look_around_timer[i] += FIXED_DT;
            float look_angle = base_angle[i] + 45.0f * my_sinf(2.0f * MA_PI * look_around_timer[i] / 2.0f);
            enemies[i].angle = look_angle;
            if (enemies[i].angle >= 360.0f) enemies[i].angle -= 360.0f;
            if (enemies[i].angle < 0.0f) enemies[i].angle += 360.0f;
        }

        // Shooting
        if ((enemies[i].state == SHOOT ||
            (enemies[i].state == TAKE_COVER && enemies[i].path_length == 0)) &&
            enemies[i].shoot_timer <= 0.0f &&
            distance < SHOOTING_RANGE) {
            Entity enemy_entity = {enemies[i].x, enemies[i].y,
                                   enemies[i].w, enemies[i].h,
                                   enemies[i].angle,
                                   {0}, 0};
            spawn_bullet(bullets, &enemy_entity, 0);
            enemies[i].shoot_timer = SHOOT_COOLDOWN;
            if (enemies[i].state == SHOOT) {
                enemies[i].decision_timer = 0.0f;
            }
        }

        // Movement along path
        move_along_path(&enemies[i], world, i, last_target_x[i], last_target_y[i]);

        // Wall collision
        float e_next_x = enemies[i].x + enemies[i].vel_x;
        float e_next_y = enemies[i].y + enemies[i].vel_y;
        bool collide_x = false, collide_y = false;
        for (int j = 0; j < world->wall_count; j++) {
            if (world->walls[j].type != WALL_NONE &&
                check_collision(e_next_x, enemies[i].y, enemies[i].w, enemies[i].h,
                                world->walls[j].x, world->walls[j].y,
                                world->walls[j].w, world->walls[j].h)) {
                collide_x = true;
            }
            if (world->walls[j].type != WALL_NONE &&
                check_collision(enemies[i].x, e_next_y, enemies[i].w, enemies[i].h,
                                world->walls[j].x, world->walls[j].y,
                                world->walls[j].w, world->walls[j].h)) {
                collide_y = true;
            }
        }
        if (!collide_x) enemies[i].x = e_next_x;
        if (!collide_y) enemies[i].y = e_next_y;

        if (enemies[i].x < 0) enemies[i].x = 0;
        if (enemies[i].x + enemies[i].w > world->w) enemies[i].x = world->w - enemies[i].w;
        if (enemies[i].y < 0) enemies[i].y = 0;
        if (enemies[i].y + enemies[i].h > world->h) enemies[i].y = world->h - enemies[i].h;

        // Check bullet hits
        for (int j = 0; j < MAX_BULLETS; j++) {
            if (bullets[j].active && bullets[j].owner == 1) {
                float bdx = bullets[j].x - (enemies[i].x + enemies[i].w / 2);
                float bdy = bullets[j].y - (enemies[i].y + enemies[i].h / 2);
                if (my_sqrt(bdx * bdx + bdy * bdy) < enemies[i].w / 2) {
                    bullets[j].active = false;
                    float damage = enemies[i].in_cover ? 5.0f : 10.0f;
                    enemies[i].hp -= damage;
                    if (enemies[i].hp <= 0.0f) {
                        enemies[i].active = false;
                        enemies[i].respawn_timer = ENEMY_RESPAWN_DELAY;
                        enemies[i].state = FREE;
                        enemies[i].in_cover = false;
                    }
                }
            }
        }
    }
}
