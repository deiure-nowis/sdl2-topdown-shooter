#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define MA_PI 3.14159265358979323846
#define WORLD_W 800
#define WORLD_H 600
#define FIXED_DT 0.016f
#define PLAYER_SPEED 300.0f
#define PLAYER_ROTATION_SPEED 240.0f // 180° turn in 0.75 seconds

typedef struct {
    float x, y, w, h;
    float vel_x, vel_y;
    float angle; // In degrees
    SDL_Texture* texture;
} Player;

// Math functions to avoid dependency issues
float my_sqrt(float x) { return sqrtf(x); }
float my_atan2f(float y, float x) { return atan2f(y, x); }
float my_cosf(float x) { return cosf(x); }
float my_sinf(float x) { return sinf(x); }
float my_fabsf(float x) { return fabsf(x); }

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Top-Down Player Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WORLD_W, WORLD_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create player texture (white rectangle)
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    if (!surface) {
        printf("SDL_CreateRGBSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
    SDL_Texture* player_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!player_texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize player
    Player player = {
        .x = WORLD_W / 2 - 16,
        .y = WORLD_H / 2 - 16,
        .w = 32,
        .h = 32,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .angle = 0.0f,
        .texture = player_texture
    };

    // Game loop variables
    int running = 1;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();
    float accumulator = 0.0f;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Calculate delta time
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        if (dt > 0.1f) dt = 0.1f; // Cap delta time to avoid large jumps
        accumulator += dt;

        // Fixed update
        while (accumulator >= FIXED_DT) {
            // Input
            const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
            float vel_input_x = 0.0f, vel_input_y = 0.0f;
            if (keyboard_state[SDL_SCANCODE_W]) vel_input_y -= PLAYER_SPEED * FIXED_DT;
            if (keyboard_state[SDL_SCANCODE_S]) vel_input_y += PLAYER_SPEED * FIXED_DT;
            if (keyboard_state[SDL_SCANCODE_A]) vel_input_x -= PLAYER_SPEED * FIXED_DT;
            if (keyboard_state[SDL_SCANCODE_D]) vel_input_x += PLAYER_SPEED * FIXED_DT;

            // Update velocity with friction
            float friction = 0.9f;
            player.vel_x += vel_input_x;
            player.vel_y += vel_input_y;
            player.vel_x *= friction;
            player.vel_y *= friction;

            // Cap speed
            float length = my_sqrt(player.vel_x * player.vel_x + player.vel_y * player.vel_y);
            if (length > PLAYER_SPEED * FIXED_DT && length > 0.0f) {
                player.vel_x = (player.vel_x / length) * PLAYER_SPEED * FIXED_DT;
                player.vel_y = (player.vel_y / length) * PLAYER_SPEED * FIXED_DT;
            }

            // Update position
            player.x += player.vel_x;
            player.y += player.vel_y;

            // Boundary checks
            if (player.x < 0) player.x = 0;
            if (player.x + player.w > WORLD_W) player.x = WORLD_W - player.w;
            if (player.y < 0) player.y = 0;
            if (player.y + player.h > WORLD_H) player.y = WORLD_H - player.h;

            // Update angle to face mouse
            int mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            float target_x = mouse_x;
            float target_y = mouse_y;
            float angle_dx = target_x - (player.x + player.w / 2);
            float angle_dy = target_y - (player.y + player.h / 2);
            float target_angle = my_atan2f(angle_dy, angle_dx) * (180.0f / MA_PI);

            // Interpolate angle with max rotation speed
            float angle_diff = target_angle - player.angle;
            while (angle_diff > 180.0f) angle_diff -= 360.0f;
            while (angle_diff < -180.0f) angle_diff += 360.0f;
            if (my_fabsf(angle_diff) < 2.0f) {
                player.angle = target_angle; // Snap to target to avoid jitter
            } else {
                float max_rotation = PLAYER_ROTATION_SPEED * FIXED_DT;
                if (angle_diff > max_rotation) angle_diff = max_rotation;
                if (angle_diff < -max_rotation) angle_diff = -max_rotation;
                player.angle += angle_diff;
                if (player.angle > 180.0f) player.angle -= 360.0f;
                if (player.angle < -180.0f) player.angle += 360.0f;
            }

            accumulator -= FIXED_DT;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect player_dst = {(int)player.x, (int)player.y, (int)player.w, (int)player.h};
        SDL_Point center = {(int)(player.w / 2), (int)(player.h / 2)};
        SDL_RenderCopyEx(renderer, player.texture, NULL, &player_dst, player.angle, &center, SDL_FLIP_NONE);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(player_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
