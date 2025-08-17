#include "common.h"
#include "types.h"
#include "utils.h"
#include "game.h"
#include "pathfinding.h"
#include "command.h"

SDL_Texture* fov_mask = NULL;

int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() < 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Game Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CAMERA_W, CAMERA_H, 0);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/Google_Sans_Code/GoogleSansCode-VariableFont_wght.ttf", 16);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* player_texture = load_texture("assets/images/16x16PlayerG.png", renderer);
    SDL_Texture* enemy_texture = load_texture("assets/images/16x16PlayerP.png", renderer);
    SDL_Texture* grass_texture = load_texture("assets/images/16x16Grass.png", renderer);
    SDL_Texture* wall_texture_S = load_texture("assets/images/16x16SWall.png", renderer);
    SDL_Texture* wall_texture_B = load_texture("assets/images/16x16Wall.png", renderer);
    SDL_Texture* wall_texture_O = load_texture("assets/images/16x16BWall.png", renderer);
    SDL_Texture* flag_texture = load_texture("assets/images/8x8FlagP.png", renderer);
    if (!player_texture || !enemy_texture || !grass_texture || !wall_texture_S || !wall_texture_B || !wall_texture_O || !flag_texture) {
        printf("Failed to load textures\n");
        SDL_DestroyTexture(player_texture);
        SDL_DestroyTexture(enemy_texture);
        SDL_DestroyTexture(grass_texture);
        SDL_DestroyTexture(wall_texture_S);
        SDL_DestroyTexture(wall_texture_B);
        SDL_DestroyTexture(wall_texture_O);
        SDL_DestroyTexture(flag_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

	GameState game_state = {true, true, true};
    World world = {0};
    world.w = WORLD_W;
    world.h = WORLD_H;
    world.background = create_background_texture(renderer, grass_texture, WORLD_W, WORLD_H);
    if (!world.background) {
        printf("Failed to create background texture\n");
        SDL_DestroyTexture(fov_mask);
        SDL_DestroyTexture(player_texture);
        SDL_DestroyTexture(enemy_texture);
        SDL_DestroyTexture(grass_texture);
        SDL_DestroyTexture(wall_texture_S);
        SDL_DestroyTexture(wall_texture_B);
        SDL_DestroyTexture(wall_texture_O);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    uint8_t map[MAP_SIZE][MAP_SIZE] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,3,0,2,2,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,1,1,2,1,1,2,1},
        {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };
    memcpy(world.map, map, sizeof(map));
    init_walls(&world, renderer, wall_texture_S, wall_texture_B, wall_texture_O);

	world.flag_count = 2; // Example: 2 flags
    world.flags = malloc(sizeof(Flag) * world.flag_count);
    if (!world.flags) {
        printf("Failed to allocate flags\n");
        SDL_DestroyTexture(fov_mask);
        SDL_DestroyTexture(player_texture);
        SDL_DestroyTexture(enemy_texture);
        SDL_DestroyTexture(grass_texture);
        SDL_DestroyTexture(wall_texture_S);
        SDL_DestroyTexture(wall_texture_B);
        SDL_DestroyTexture(wall_texture_O);
        SDL_DestroyTexture(flag_texture);
        SDL_DestroyTexture(world.background);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    for (int i = 0; i < world.flag_count; i++) {
        world.flags[i].x = (MAP_SIZE / 4 + (i * MAP_SIZE / 2)) * TILE_SIZE;
        world.flags[i].y = (MAP_SIZE / 4) * TILE_SIZE;
        world.flags[i].w = 24;
        world.flags[i].h = 24;
        world.flags[i].texture = flag_texture;
        world.flags[i].active = true;
        world.flags[i].enemy_count = 5; // Default 5 enemies per flag
        world.flags[i].spawn_timer = 0.0f;
        world.flags[i].enemies_spawned = 0;
    }

    Player player = {WORLD_W / 2, WORLD_H / 2, 48, 48, 0, 0, 0, player_texture};
    Camera camera = {WORLD_W / 2 - CAMERA_W / 2, WORLD_H / 2 - CAMERA_H / 2, CAMERA_W, CAMERA_H};
    Bullet bullets[MAX_BULLETS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};
    Console console = {0};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].texture = enemy_texture;
        enemies[i].flag_id = -1;
    }

	// Initial enemy spawn around flags
    if (game_state.spawn_enabled) {
        int total_enemies_spawned = 0;
        for (int i = 0; i < world.flag_count && total_enemies_spawned < MAX_ENEMIES; i++) {
            if (!world.flags[i].active) continue;
            for (int j = 0; j < world.flags[i].enemy_count && total_enemies_spawned < MAX_ENEMIES; j++) {
                for (int k = 0; k < MAX_ENEMIES; k++) {
                    if (!enemies[k].active) {
                        spawn_enemy(&enemies[k], &world, &camera, i);
                        world.flags[i].enemies_spawned++;
                        total_enemies_spawned++;
                        break;
                    }
                }
            }
            if (total_enemies_spawned >= MAX_ENEMIES) {
                char msg[MAX_COMMAND_LENGTH];
                snprintf(msg, MAX_COMMAND_LENGTH, "Warning: MAX_ENEMIES (%d) reached, some enemies not spawned", MAX_ENEMIES);
                printf("%s\n", msg);
                // Log to console
                Console temp_console = {0};
                init_console(&temp_console, renderer, font);
                my_strcpy(temp_console.text[temp_console.current_line], msg);
                temp_console.timestamps[temp_console.current_line] = MESSAGE_DURATION;
                temp_console.current_line = (temp_console.current_line + 1) % MAX_CONSOLE_LINES;
                temp_console.line_count = temp_console.line_count < MAX_CONSOLE_LINES ? temp_console.line_count + 1 : MAX_CONSOLE_LINES;
            }
        }
    }

    init_console(&console, renderer, font);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetWindowFullscreen(window, game_state.is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_StartTextInput();

    bool running = true;
    float frame_times[10] = {0.0f};
    int frame_time_index = 0;
    float fps = 0.0f;
    float fps_update_timer = 0.0f;
    float accumulator = 0.0f;

	const Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 last_time = SDL_GetPerformanceCounter();

    while (running) {
		Uint64 frame_start = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (console.active) {
                    // Only handle console-related keys when console is active
                    handle_console_input(&console, &player, &world, &event);
                } else {
                    // Handle game controls only when console is inactive
                    switch (key) {
                        case SDLK_m:
                            game_state.minimap = !game_state.minimap;
                            break;
                        case SDLK_k:
                            game_state.is_fullscreen = !game_state.is_fullscreen;
                            SDL_SetWindowFullscreen(window, game_state.is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                            break;
                        case SDLK_ESCAPE:
                            printf("ESC\n");
                            running = false;
                            break;
                        case SDLK_RETURN:
                            handle_console_input(&console, &player, &world, &event);
                            break;
                        default:
							break;
                    }
                }
            } else if (event.type == SDL_TEXTINPUT && console.active) {
                handle_console_input(&console, &player, &world, &event);
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !console.active) {
                Entity player_entity = {player.x, player.y, player.w, player.h, player.angle};
                spawn_bullet(bullets, &player_entity, 1);
            }
        }

        Uint64 current_time = SDL_GetPerformanceCounter();
        float delta_time = (float)(current_time - last_time) / frequency;
        last_time = current_time;

        accumulator += delta_time;
        while (accumulator >= FIXED_DT) {
			fixed_update(&player, &world, bullets, enemies, &camera, &console, &game_state);
            accumulator -= FIXED_DT;
        }
        update_camera(&camera, &player, &world, renderer);
        render(renderer, &player, &camera, &world, bullets, enemies, font, fps, &console, &game_state);

		// Frame rate capping
        Uint64 end_time = SDL_GetPerformanceCounter();
        float frame_time_ms = (float)(end_time - frame_start) * 1000.0f / frequency;
        if (frame_time_ms < TARGET_FRAME_TIME) {
            SDL_Delay((Uint32)(TARGET_FRAME_TIME - frame_time_ms));
        }

		// Update FPS calculation to include delay
		fps_update_timer += delta_time;
        frame_times[frame_time_index] = delta_time;
        frame_time_index = (frame_time_index + 1) % 10;
        if (fps_update_timer >= FPS_UPDATE_INTERVAL) {
            float avg_frame_time = 0.0f;
            for (int i = 0; i < 10; i++) {
                avg_frame_time += frame_times[i];
            }
            avg_frame_time /= 10.0f;
            fps = avg_frame_time > 0.0f ? 1.0f / avg_frame_time : 0.0f;
            fps_update_timer = 0.0f;
        }
    }

    free_console(&console);
    for (int i = 0; i < world.wall_count; i++) SDL_DestroyTexture(world.walls[i].texture);
    free(world.walls);
    SDL_DestroyTexture(player_texture);
    SDL_DestroyTexture(enemy_texture);
    SDL_DestroyTexture(grass_texture);
    SDL_DestroyTexture(wall_texture_S);
    SDL_DestroyTexture(wall_texture_B);
    SDL_DestroyTexture(wall_texture_O);
    SDL_DestroyTexture(flag_texture);
    SDL_DestroyTexture(world.background);
    SDL_DestroyTexture(fov_mask);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_StopTextInput();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
