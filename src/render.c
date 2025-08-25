#include "render.h"
#include "utils.h"
#include "command.h"
#include "menu.h"

// Renders a minimap to the screen showing walls, player, bullets, enemies, and camera view.
void render_minimap(SDL_Renderer* renderer, Player* player, Bullet* bullets, Camera* camera, World* world, Enemy* enemies) {
    // Safety check: if renderer is null, print an error and exit the function.
    if (!renderer) {
        printf("Error: Null renderer in render_minimap\n");
        return;
    }

    // Define dimensions and position of the minimap on the screen.
    int minimap_w = WORLD_W / 8, minimap_h = WORLD_H / 8;
    int minimap_x = camera->w - minimap_w - 10, minimap_y = 10;

    // Calculate scaling factor between world size and minimap size.
    float scale = (float)minimap_w / world->w;

    // Draw the minimap background (semi-transparent blue).
    SDL_SetRenderDrawColor(renderer, 0, 55, 200, 155);
    SDL_Rect minimap_rect = {minimap_x, minimap_y, minimap_w, minimap_h};
    SDL_RenderFillRect(renderer, &minimap_rect);

    // Draw all walls on the minimap.
    for (int i = 0; i < world->wall_count; i++) {
        if (world->walls[i].type != WALL_NONE) {
            SDL_Rect wall_rect = {
                minimap_x + (int)(world->walls[i].x * scale),
                minimap_y + (int)(world->walls[i].y * scale),
                (int)(world->walls[i].w * scale),
                (int)(world->walls[i].h * scale)
            };
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // gray
            SDL_RenderFillRect(renderer, &wall_rect);
        }
    }

    // Draw the player as a white rectangle.
    SDL_Rect player_rect = {
        minimap_x + (int)(player->x * scale),
        minimap_y + (int)(player->y * scale),
        (int)(player->w * scale),
        (int)(player->h * scale)
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
    SDL_RenderFillRect(renderer, &player_rect);

    // Draw all active bullets as small yellow squares (2x2).
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect bullet_rect = {
                minimap_x + (int)(bullets[i].x * scale),
                minimap_y + (int)(bullets[i].y * scale),
                2, 2
            };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow
            SDL_RenderFillRect(renderer, &bullet_rect);
        }
    }

    // Draw all active enemies. Cyan if in cover, magenta if exposed.
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            SDL_Rect enemy_rect = {
                minimap_x + (int)(enemies[i].x * scale),
                minimap_y + (int)(enemies[i].y * scale),
                (int)(enemies[i].w * scale),
                (int)(enemies[i].h * scale)
            };
            if(enemies[i].in_cover){
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // cyan
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // magenta
            }
            SDL_RenderFillRect(renderer, &enemy_rect);
        }
    }

	for (int i = 0; i < world->flag_count; i++) {
        if (world->flags[i].active) {
            SDL_Rect flag_rect = {
                minimap_x + (int)(world->flags[i].x * scale),
                minimap_y + (int)(world->flags[i].y * scale),
                (int)(world->flags[i].w * scale),
                (int)(world->flags[i].h * scale)
            };
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for flags
            SDL_RenderFillRect(renderer, &flag_rect);
        }
    }

    // Draw the camera viewport as a red rectangle on the minimap.
    SDL_Rect camera_rect = {
        minimap_x + (int)(camera->x * scale),
        minimap_y + (int)(camera->y * scale),
        (int)(camera->w * scale),
        (int)(camera->h * scale)
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
    SDL_RenderDrawRect(renderer, &camera_rect);
}

void init_fov_mask(SDL_Renderer* renderer, SDL_Texture** fov_mask, int w, int h) {
    if (*fov_mask) {
        SDL_DestroyTexture(*fov_mask);
        *fov_mask = NULL;
    }
    *fov_mask = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!*fov_mask) {
        printf("Failed to create FOV mask texture: %s\n", SDL_GetError());
        return;
    }
    SDL_SetTextureBlendMode(*fov_mask, SDL_BLENDMODE_BLEND);
}

void render_fov(SDL_Renderer* renderer, Player* player, Camera* camera, World* world, SDL_Texture* fov_mask) {
    if (!renderer || !fov_mask) {
        printf("Error: Null renderer or FOV mask in render_fov\n");
        return;
    }

    // Set the render target to the FOV mask texture
    SDL_SetRenderTarget(renderer, fov_mask);
    
    // Clear with gray fog (semi-transparent)
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, FOV_GRAY_ALPHA);
    SDL_RenderClear(renderer);

    // Save the current blend mode
    SDL_BlendMode current_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &current_blend_mode);

    // Set blend mode to NONE to overwrite pixels with transparency
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Fully transparent

    // Calculate player position in world and screen coordinates
    float player_x = player->x + player->w / 2.0f;
    float player_y = player->y + player->h / 2.0f;
    float player_screen_x = player_x - camera->x;
    float player_screen_y = player_y - camera->y;

    // Array to hold points for polygons (sector + circle, each with 64 rays + start/end points)
    SDL_Point points[66]; // 64 rays + player start + player end

    // --- Sector Polygon (90-degree, 700 pixels) ---
    int rays = 64;
    float angle_rad = player->angle * (MA_PI / 180.0f);
    float half_fov_rad = FOV_HALF_ANGLE * (MA_PI / 180.0f);
    float angle_step = (2.0f * half_fov_rad) / (rays - 1);

    points[0].x = (int)player_screen_x;
    points[0].y = (int)player_screen_y;
    for (int i = 0; i < rays; i++) {
        float ray_angle = angle_rad - half_fov_rad + angle_step * i;
        float cos_a = my_cosf(ray_angle);
        float sin_a = my_sinf(ray_angle);
        float dist = get_visibility_distance(player_x, player_y, cos_a, sin_a, world);
        if (dist > FOV_RANGE) dist = FOV_RANGE;
        points[i + 1].x = (int)(player_screen_x + cos_a * dist);
        points[i + 1].y = (int)(player_screen_y + sin_a * dist);
    }
    points[rays + 1].x = points[0].x;
    points[rays + 1].y = points[0].y;
    SDL_RenderFillPolygon(renderer, points, rays + 2);

    // --- Circle Polygon (360-degree, 64 pixels) ---
    float circle_angle_step = (2.0f * MA_PI) / (rays - 1);
    points[0].x = (int)player_screen_x;
    points[0].y = (int)player_screen_y;
    for (int i = 0; i < rays; i++) {
        float ray_angle = circle_angle_step * i;
        float cos_a = my_cosf(ray_angle);
        float sin_a = my_sinf(ray_angle);
        float dist = get_visibility_distance(player_x, player_y, cos_a, sin_a, world);
        if (dist > FOV_CIRCLE_R) dist = FOV_CIRCLE_R;
        points[i + 1].x = (int)(player_screen_x + cos_a * dist);
        points[i + 1].y = (int)(player_screen_y + sin_a * dist);
    }
    points[rays + 1].x = points[0].x;
    points[rays + 1].y = points[0].y;
    SDL_RenderFillPolygon(renderer, points, rays + 2);

    // Restore original blend mode and render target
    SDL_SetRenderDrawBlendMode(renderer, current_blend_mode);
    SDL_SetRenderTarget(renderer, NULL);
}

bool is_in_fov(float x, float y, Player* player, World* world, float* alpha) {
    float player_x = player->x + player->w / 2.0f;
    float player_y = player->y + player->h / 2.0f;
    float dx = x - player_x;
    float dy = y - player_y;
    float distance = my_sqrt(dx * dx + dy * dy);

    bool in_sector = false;
    bool in_circle = false;

    // Check 360-degree circle (FOV_CIRCLE_R = 64.0f)
    if (distance <= FOV_CIRCLE_R) {
        in_circle = true;
    }

    // Check 90-degree sector (FOV_RANGE = 700.0f)
    if (distance <= FOV_RANGE) {
        float angle_to_point = my_atan2f(dy, dx) * (180.0f / MA_PI);
        float angle_diff = angle_to_point - player->angle;
        while (angle_diff > 180.0f) angle_diff -= 360.0f;
        while (angle_diff < -180.0f) angle_diff += 360.0f;
        if (absf(angle_diff) <= FOV_HALF_ANGLE) {
            in_sector = true;
        }
    }

    // Point is visible if in either region
    if (!in_sector && !in_circle) {
        *alpha = 0.0f;
        return false;
    }

    // Check line-of-sight (blocked by opaque walls)
    if (!has_line_of_sight(player_x, player_y, x, y, world, false, true)) {
        *alpha = 0.0f;
        return false;
    }

    // Calculate alpha based on the region
    if (in_circle) {
        // Fade in the circle's transitional range (54 to 64 pixels)
        if (distance > FOV_CIRCLE_TRANSITIONAL_RANGE) {
            *alpha = 255.0f * (FOV_CIRCLE_R - distance) / (FOV_CIRCLE_R - FOV_CIRCLE_TRANSITIONAL_RANGE);
        } else {
            *alpha = 255.0f;
        }
    } else {
        // Fade in the sector's transitional range (650 to 700 pixels)
        if (distance > FOV_TRANSITIONAL_RANGE) {
            *alpha = 255.0f * (FOV_RANGE - distance) / (FOV_RANGE - FOV_TRANSITIONAL_RANGE);
        } else {
            *alpha = 255.0f;
        }
    }

    return true;
}

// Modified render() function in render.c (only the relevant parts are shown; replace accordingly)

void render(SDL_Renderer* renderer, Player* player, Camera* camera, World* world, Bullet* bullets, Enemy* enemies, TTF_Font* font, Console* console, GameState* game_state, Menu* menu) {
    if (!renderer) {
        printf("Error: Null renderer in render\n");
        return;
    }

    // Check renderer size and recreate fov_mask if necessary
    static int last_w = 0, last_h = 0;
    int renderer_w, renderer_h;
    if (SDL_GetRendererOutputSize(renderer, &renderer_w, &renderer_h) != 0) {
        printf("Error: SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
        renderer_w = CAMERA_W;
        renderer_h = CAMERA_H;
    }
    if (renderer_w != last_w || renderer_h != last_h || !fov_mask) {
        last_w = renderer_w;
        last_h = renderer_h;
        init_fov_mask(renderer, &fov_mask, renderer_w, renderer_h);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect bg_dst = {-(int)camera->x, -(int)camera->y, WORLD_W, WORLD_H};
    SDL_RenderCopy(renderer, world->background, NULL, &bg_dst);

    // Render walls
    for (int i = 0; i < world->wall_count; i++) {
        if (world->walls[i].type != WALL_NONE) {
            SDL_Rect wall_rect = {
                (int)(world->walls[i].x - camera->x),
                (int)(world->walls[i].y - camera->y),
                (int)world->walls[i].w,
                (int)world->walls[i].h
            };
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            if (world->walls[i].type == WALL_BULLETPROOF) {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            } else if (world->walls[i].type == WALL_OPAQUE) {
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            }
            SDL_SetTextureAlphaMod(world->walls[i].texture, 255);
            SDL_RenderCopy(renderer, world->walls[i].texture, NULL, &wall_rect);
        }
    }

    // Render flags
    for (int i = 0; i < world->flag_count; i++) {
        if (world->flags[i].active) {
            SDL_Rect flag_rect = {
                (int)(world->flags[i].x - camera->x),
                (int)(world->flags[i].y - camera->y),
                (int)world->flags[i].w,
                (int)world->flags[i].h
            };
            SDL_SetTextureAlphaMod(world->flags[i].texture, 255);
            SDL_RenderCopy(renderer, world->flags[i].texture, NULL, &flag_rect);
        }
    }

    // Render player (always visible)
    SDL_Rect player_dst_rect = {(int)(player->x - camera->x), (int)(player->y - camera->y),
                                (int)player->w, (int)player->h};
    SDL_Point pcenter = {(int)player->w / 2, (int)player->h / 2};
    SDL_RenderCopyEx(renderer, player->texture, NULL, &player_dst_rect, player->angle, &pcenter, SDL_FLIP_NONE);

    // Render enemies with FOV and transitional alpha
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float alpha;
            float enemy_center_x = enemies[i].x + enemies[i].w / 2;
            float enemy_center_y = enemies[i].y + enemies[i].h / 2;
            if (!is_in_fov(enemy_center_x, enemy_center_y, player, world, &alpha)) {
                continue;
            }
            SDL_Rect enemy_dst_rect = {
                (int)(enemies[i].x - camera->x),
                (int)(enemies[i].y - camera->y),
                (int)enemies[i].w,
                (int)enemies[i].h
            };
            SDL_Point center = {(int)(enemies[i].w / 2), (int)(enemies[i].h / 2)};
            SDL_SetTextureAlphaMod(enemies[i].texture, (Uint8)alpha);
            SDL_RenderCopyEx(renderer, enemies[i].texture, NULL, &enemy_dst_rect, enemies[i].angle, &center, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(enemies[i].texture, 255);
        }
    }

    // Render bullets with FOV
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            float alpha = 255;
            //if (!is_in_fov(bullets[i].x, bullets[i].y, player, world, &alpha)) continue;
            SDL_Rect bullet_rect = {(int)(bullets[i].x - camera->x),
                                    (int)(bullets[i].y - camera->y), 5, 5};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, (Uint8)alpha);
            SDL_RenderFillRect(renderer, &bullet_rect);
        }
    }

    // Render FOV mask
    render_fov(renderer, player, camera, world, fov_mask);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(fov_mask, 255);
    SDL_RenderCopy(renderer, fov_mask, NULL, NULL);

    static SDL_Texture* fps_texture = NULL;
    static float last_fps = -1.0f;
    if (font && absf(world->fps - last_fps) > 1.0f) {
        last_fps = world->fps;
        if (fps_texture) SDL_DestroyTexture(fps_texture);
        char fps_text[16];
        snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", world->fps);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, fps_text, white);
        if (text_surface) {
            fps_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_FreeSurface(text_surface);
        }
    }
    if (fps_texture) {
        int text_w, text_h;
        SDL_QueryTexture(fps_texture, NULL, NULL, &text_w, &text_h);
        SDL_Rect text_rect = {10, 10, text_w, text_h};
        SDL_RenderCopy(renderer, fps_texture, NULL, &text_rect);
    }

    render_console(renderer, console, font);

    if (game_state->minimap) render_minimap(renderer, player, bullets, camera, world, enemies);
    render_menu(renderer, menu);
    SDL_RenderPresent(renderer);
}
