#include "menu.h"
#include "utils.h"

void init_menu(Menu* menu, SDL_Renderer* renderer, TTF_Font* font, GameState* game_state) {
	if (!renderer || !font || !game_state) {
		printf("Error: Null renderer, font, or game_state in init_menu\n");
		return;
	}
	menu->active = false;
	menu->type = MAIN_MENU;
	menu->selected_option = -1;
	menu->game_state = game_state;
	menu->font = font;

	// Initialize Main Menu
	const char* main_option_texts[MENU_OPTION_COUNT] = {"Resume", "Options", "Quit", ""};
	int menu_width = 200;
	int option_height = 50;
	int spacing = 10;
	int renderer_w, renderer_h;
	if (SDL_GetRendererOutputSize(renderer, &renderer_w, &renderer_h) != 0) {
		printf("Error: SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
		renderer_w = CAMERA_W;
		renderer_h = CAMERA_H;
	}
	int total_height = 3 * option_height + 2 * spacing; // 3 options for main menu
	int start_y = (renderer_h - total_height) / 2;

	for (int i = 0; i < MENU_OPTION_COUNT; i++) {
		MenuOption* opt = &menu->main_options[i];
		if (i < 3) { // Only first 3 slots used for main menu
			my_strcpy(opt->text, main_option_texts[i]);
		} else {
			opt->text[0] = '\0'; // Empty for unused slot
		}
		opt->rect.x = (renderer_w - menu_width) / 2;
		opt->rect.y = start_y + i * (option_height + spacing);
		opt->rect.w = menu_width;
		opt->rect.h = i < 3 ? option_height : 0; // Zero height for unused
		opt->is_hovered = false;

		if (opt->text[0]) {
			SDL_Color text_color = {255, 255, 255, 255};
			SDL_Surface* surface = TTF_RenderText_Solid(font, opt->text, text_color);
			if (!surface) {
				printf("Failed to render main menu text '%s': %s\n", opt->text, TTF_GetError());
				opt->text_texture = NULL;
				continue;
			}
			opt->text_texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
			if (!opt->text_texture) {
				printf("Failed to create main menu text texture for '%s': %s\n", opt->text, SDL_GetError());
			}
		} else {
			opt->text_texture = NULL;
		}
	}

	// Initialize Options Menu
	total_height = MENU_OPTION_COUNT * option_height + (MENU_OPTION_COUNT - 1) * spacing;
	start_y = (renderer_h - total_height) / 2;
	for (int i = 0; i < MENU_OPTION_COUNT; i++) {
		MenuOption* opt = &menu->options_menu_options[i];
		opt->rect.x = (renderer_w - menu_width) / 2;
		opt->rect.y = start_y + i * (option_height + spacing);
		opt->rect.w = menu_width;
		opt->rect.h = option_height;
		opt->is_hovered = false;
		opt->text_texture = NULL; // Created dynamically
	}
}

void handle_menu_input(Menu* menu, SDL_Event* event, bool* running, SDL_Window* window) {
    if (!menu || !event || !running) return;
    if (!menu->active) return;

    MenuOption* options = (menu->type == MAIN_MENU) ? menu->main_options : menu->options_menu_options;
    int option_count = (menu->type == MAIN_MENU) ? 3 : MENU_OPTION_COUNT;

    switch (event->type) {
        case SDL_MOUSEMOTION: {
            int mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            menu->selected_option = -1;
            for (int i = 0; i < option_count; i++) {
                MenuOption* opt = &options[i];
                bool is_hovered = (mouse_x >= opt->rect.x && mouse_x < opt->rect.x + opt->rect.w &&
                                   mouse_y >= opt->rect.y && mouse_y < opt->rect.y + opt->rect.h);
                opt->is_hovered = is_hovered;
                if (is_hovered) menu->selected_option = i;
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT && menu->selected_option >= 0) {
                if (menu->type == MAIN_MENU) {
                    switch (menu->selected_option) {
                        case 0: // Resume
                            menu->active = false;
                            menu->selected_option = -1;
                            break;
                        case 1: // Options
                            menu->type = OPTIONS_MENU;
                            menu->selected_option = -1;
                            break;
                        case 2: // Quit
                            *running = false;
                            break;
                    }
                } else { // OPTIONS_MENU
                    switch (menu->selected_option) {
                        case 0: // Minimap
                            menu->game_state->minimap = !menu->game_state->minimap;
                            break;
                        case 1: // Fullscreen
                            menu->game_state->is_fullscreen = !menu->game_state->is_fullscreen;
                            SDL_SetWindowFullscreen(window, menu->game_state->is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                            break;
                        case 2: // Spawn Enemies
                            menu->game_state->spawn_enabled = !menu->game_state->spawn_enabled;
                            break;
                        case 3: // Back
                            menu->type = MAIN_MENU;
                            menu->selected_option = -1;
                            break;
                    }
                }
            }
            break;
    }
}

void render_menu(SDL_Renderer* renderer, Menu* menu) {
    if (!menu->active) return;
    if (!renderer || !menu->font) return;

    // Get current renderer size
    int renderer_w, renderer_h;
    if (SDL_GetRendererOutputSize(renderer, &renderer_w, &renderer_h) != 0) {
        printf("Error: SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
        renderer_w = CAMERA_W;
        renderer_h = CAMERA_H;
    }

    // Update option positions
    int menu_width = 200;
    int option_height = 50;
    int spacing = 10;
    int count = (menu->type == MAIN_MENU) ? 3 : MENU_OPTION_COUNT;
    int total_height = count * option_height + (count - 1) * spacing;
    int start_y = (renderer_h - total_height) / 2;

    MenuOption* options = (menu->type == MAIN_MENU) ? menu->main_options : menu->options_menu_options;
    for (int i = 0; i < count; i++) {
        MenuOption* opt = &options[i];
        opt->rect.x = (renderer_w - menu_width) / 2;
        opt->rect.y = start_y + i * (option_height + spacing);
        opt->rect.w = menu_width;
        opt->rect.h = option_height;
    }

    // Update Options menu text textures
    if (menu->type == OPTIONS_MENU) {
        const char* option_texts[MENU_OPTION_COUNT] = {
            menu->game_state->minimap ? "Minimap: On" : "Minimap: Off",
            menu->game_state->is_fullscreen ? "Fullscreen: On" : "Fullscreen: Off",
            menu->game_state->spawn_enabled ? "Spawn Enemies: On" : "Spawn Enemies: Off",
            "Back"
        };
        for (int i = 0; i < MENU_OPTION_COUNT; i++) {
            MenuOption* opt = &menu->options_menu_options[i];
            if (my_strcmp(opt->text, option_texts[i]) != 0) {
                my_strcpy(opt->text, option_texts[i]);
                if (opt->text_texture) {
                    SDL_DestroyTexture(opt->text_texture);
                    opt->text_texture = NULL;
                }
                SDL_Color text_color = {255, 255, 255, 255};
                SDL_Surface* surface = TTF_RenderText_Solid(menu->font, opt->text, text_color);
                if (surface) {
                    opt->text_texture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                    if (!opt->text_texture) {
                        printf("Failed to create options menu text texture for '%s': %s\n", opt->text, SDL_GetError());
                    }
                } else {
                    printf("Failed to render options menu text '%s': %s\n", opt->text, TTF_GetError());
                }
            }
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < count; i++) {
        MenuOption* opt = &options[i];
        if (!opt->rect.h) continue; // Skip unused slots

        // Background rectangle
        SDL_SetRenderDrawColor(renderer, opt->is_hovered ? 255 : 0, opt->is_hovered ? 255 : 0, opt->is_hovered ? 255 : 0, 255);
        SDL_RenderFillRect(renderer, &opt->rect);

        // Border
        SDL_SetRenderDrawColor(renderer, opt->is_hovered ? 0 : 255, opt->is_hovered ? 0 : 255, opt->is_hovered ? 0 : 255, 255);
        SDL_RenderDrawRect(renderer, &opt->rect);

        // Text
        if (opt->text_texture) {
            int text_w, text_h;
            if (SDL_QueryTexture(opt->text_texture, NULL, NULL, &text_w, &text_h) == 0) {
                SDL_Rect text_rect = {
                    opt->rect.x + (opt->rect.w - text_w) / 2,
                    opt->rect.y + (opt->rect.h - text_h) / 2,
                    text_w,
                    text_h
                };
                SDL_SetTextureColorMod(opt->text_texture, opt->is_hovered ? 0 : 255, opt->is_hovered ? 0 : 255, opt->is_hovered ? 0 : 255);
                SDL_RenderCopy(renderer, opt->text_texture, NULL, &text_rect);
            } else {
                printf("Failed to query texture for '%s': %s\n", opt->text, SDL_GetError());
            }
        }
    }
}

void free_menu(Menu* menu) {
	for (int i = 0; i < MENU_OPTION_COUNT; i++) {
		if (menu->main_options[i].text_texture) {
			SDL_DestroyTexture(menu->main_options[i].text_texture);
			menu->main_options[i].text_texture = NULL;
		}
		if (menu->options_menu_options[i].text_texture) {
			SDL_DestroyTexture(menu->options_menu_options[i].text_texture);
			menu->options_menu_options[i].text_texture = NULL;
		}
	}
}
