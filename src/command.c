#include "command.h"
#include "utils.h"
#include "common.h"

void init_console(Console* console, SDL_Renderer* renderer, TTF_Font* font) {
    console->line_count = 0;
    console->current_line = 0;
    console->input_length = 0;
    console->active = false;
    console->input[0] = '\0';
    console->suggestion[0] = '\0';
    console->suggestion_texture = NULL;
    console->history_count = 0;
    console->history_index = -1;
    console->cursor_pos = 0;
    for (int i = 0; i < MAX_CONSOLE_LINES; i++) {
        console->text[i][0] = '\0';
        console->timestamps[i] = 0.0f;
        console->console_textures[i] = NULL;
    }
    for (int i = 0; i < MAX_HISTORY; i++) {
        console->history[i][0] = '\0';
    }
    console->input_texture = NULL;
}

void compute_suggestion(Console* console) {
    // List of available commands
    const char* commands[] = {"say", "tp"};
    console->suggestion[0] = '\0';

    // Find matching command
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (my_strncmp(console->input, commands[i], console->input_length) == 0) {
            if (my_strlen(commands[i]) > console->input_length) {
                my_strcpy(console->suggestion, commands[i] + console->input_length);
            }
            break;
        }
    }
}

void handle_console_input(Console* console, Player* player, World* world, SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        SDL_Keycode key = event->key.keysym.sym;
        if (key == SDLK_RETURN) {
            if (!console->active) {
                console->active = true;
                console->input[0] = '\0';
                console->input_length = 0;
                console->cursor_pos = 0;
                console->suggestion[0] = '\0';
                console->history_index = -1;
                SDL_StartTextInput();
                return;
            } else if (console->input_length > 0) {
                execute_command(console, player, world, console->input);
                console->input[0] = '\0';
                console->input_length = 0;
                console->cursor_pos = 0;
                console->suggestion[0] = '\0';
                console->history_index = -1;
                console->active = false;
                SDL_StopTextInput();
                return;
            }
        }
        if (!console->active) return;

        if (key == SDLK_BACKSPACE && console->input_length > 0) {
            for(int i=console->cursor_pos - 1; i < console->input_length; i++) console->input[i] = console->input[i + 1];
            console->input_length--;
            console->cursor_pos--;
            console->history_index = -1;
            compute_suggestion(console);
        } else if (key == SDLK_LEFT && console->cursor_pos > 0) {
            // Move cursor left
            console->cursor_pos--;
            console->history_index = -1;
            compute_suggestion(console);
        } else if (key == SDLK_RIGHT && console->cursor_pos < console->input_length) {
            // Move cursor right
            console->cursor_pos++;
            console->history_index = -1;
            compute_suggestion(console);
        } else if (key == SDLK_TAB && console->suggestion[0] != '\0') {
			// Accept suggestion
			my_strcat(console->input, console->suggestion);
			console->input_length += my_strlen(console->suggestion);
			console->cursor_pos = console->input_length;
			console->suggestion[0] = '\0';
			console->history_index = -1;
        } else if (key == SDLK_UP && console->history_count > 0) {
            // Navigate backward in history
            if (console->history_index == -1) {
                console->history_index = console->history_count - 1;
            } else if (console->history_index > 0) {
                console->history_index--;
            }
            if (console->history_index >= 0) {
                strcpy(console->input, console->history[console->history_index]);
                console->input_length = strlen(console->input);
                console->cursor_pos = console->input_length;
                compute_suggestion(console);
            }
        } else if (key == SDLK_DOWN && console->history_count > 0) {
            // Navigate forward in history
            if (console->history_index >= 0) {
                console->history_index++;
                if (console->history_index >= console->history_count) {
                    console->history_index = -1;
                    console->input[0] = '\0';
                    console->input_length = 0;
                    console->cursor_pos = 0;
                    console->suggestion[0] = '\0';
                } else {
                    strcpy(console->input, console->history[console->history_index]);
                    console->input_length = strlen(console->input);
                    console->cursor_pos = console->input_length;
                    compute_suggestion(console);
                }
            }
        } else if (key == SDLK_ESCAPE){
			console->active = false;
			SDL_StopTextInput();
		}
    } else if (event->type == SDL_TEXTINPUT && console->active) {
        if (console->input_length < MAX_COMMAND_LENGTH - 1) {
			for(int i=console->input_length; i>console->cursor_pos; i--) console->input[i] = console->input[i - 1];
            console->input[console->cursor_pos] = event->text.text[0];
            console->input_length++;
            console->cursor_pos++;
            console->input[console->input_length] = '\0';
            console->history_index = -1;
            compute_suggestion(console);
        }
    }
}

void execute_command(Console* console, Player* player, World* world, const char* command) {
    char cmd[MAX_COMMAND_LENGTH];
    char arg[MAX_COMMAND_LENGTH];
    float x = 0.0f, y = 0.0f;

	if(console->history_count < MAX_HISTORY && command[0] != '\0'){
		my_strcpy(console->history[console->history_count], command);
		console->history_count++;
	}else if(command[0] != '\0'){
		for(int i=1; i<MAX_HISTORY; i++) my_strcpy(console->history[i - 1], console->history[i]);
		my_strcpy(console->history[MAX_HISTORY - 1], command);
	}

    // Parse command (trim spaces for robustness)
    cmd[0] = '\0';
    arg[0] = '\0';
    sscanf(command, "%s %[^\n]", cmd, arg);
    // Trim trailing spaces from cmd
    for (int i = my_strlen(cmd) - 1; i >= 0 && cmd[i] == ' '; i--) {
        cmd[i] = '\0';
    }

    if (my_strcmp(cmd, "say") == 0) {
        if (my_strlen(arg) > 0) {
            if (console->line_count < MAX_CONSOLE_LINES) {
                my_strcpy(console->text[console->line_count], arg);
                console->timestamps[console->line_count] = 0.0f;
                console->line_count++;
                if (console->line_count > MAX_CONSOLE_LINES - 1) {
                    for (int i = 1; i < MAX_CONSOLE_LINES; i++) {
                        my_strcpy(console->text[i - 1], console->text[i]);
                        console->timestamps[i - 1] = console->timestamps[i];
                    }
                    console->line_count = MAX_CONSOLE_LINES - 1;
                }
            }
        } else {
            if (console->line_count < MAX_CONSOLE_LINES) {
                my_strcpy(console->text[console->line_count], "Say command requires a message");
                console->timestamps[console->line_count] = 0.0f;
                console->line_count++;
            }
        }
    } else if (my_strcmp(cmd, "tp") == 0) {
        if (player == NULL) {
            if (console->line_count < MAX_CONSOLE_LINES) {
                my_strcpy(console->text[console->line_count], "Error: Player not available");
                console->timestamps[console->line_count] = 0.0f;
                console->line_count++;
            }
            return;
        }
        int parsed = sscanf(command, "%*s %f %f", &x, &y);
        if (parsed == 2) {
            // Validate coordinates
            if (x >= 0 && x < WORLD_W && y >= 0 && y < WORLD_H) {
				bool colide = false;
				for(int i=0; i < world->wall_count && !colide; i++){
					if(world->walls[i].type != WALL_NONE && check_collision(x, y, player->w, player->h, world->walls[i].x, world->walls[i].y,
							world->walls[i].w, world->walls[i].h)){
						colide = true;
						if (console->line_count < MAX_CONSOLE_LINES) {
							my_strcpy(console->text[console->line_count], "Error: Invalid coordinates");
							console->timestamps[console->line_count] = 0.0f;
							console->line_count++;
						}
					}
				}
				if(!colide){
					// Basic collision check (optional, can be expanded)
					player->x = x;
					player->y = y;
					char msg[256];
					snprintf(msg, sizeof(msg), "Teleported to %.0f, %.0f", x, y);
					if (console->line_count < MAX_CONSOLE_LINES) {
						my_strcpy(console->text[console->line_count], msg);
						console->timestamps[console->line_count] = 0.0f;
						console->line_count++;
						if (console->line_count > MAX_CONSOLE_LINES - 1) {
							for (int i = 1; i < MAX_CONSOLE_LINES; i++) {
								my_strcpy(console->text[i - 1], console->text[i]);
								console->timestamps[i - 1] = console->timestamps[i];
							}
							console->line_count = MAX_CONSOLE_LINES - 1;
						}
					}
				}
            } else {
                if (console->line_count < MAX_CONSOLE_LINES) {
                    my_strcpy(console->text[console->line_count], "Error: Invalid coordinates");
                    console->timestamps[console->line_count] = 0.0f;
                    console->line_count++;
                }
            }
        } else {
            if (console->line_count < MAX_CONSOLE_LINES) {
                my_strcpy(console->text[console->line_count], "Error: Invalid tp command format - use: tp [x] [y]");
                console->timestamps[console->line_count] = 0.0f;
                console->line_count++;
            }
        }
    } else {
        if (console->line_count < MAX_CONSOLE_LINES) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Unknown command: %s", cmd);
            my_strcpy(console->text[console->line_count], msg);
            console->timestamps[console->line_count] = 0.0f;
            console->line_count++;
        }
    }
}

void render_console(SDL_Renderer* renderer, Console* console, TTF_Font* font) {
    static float cursor_timer = 0.0f;

    // Update cursor timer
    cursor_timer += FIXED_DT;
    if (cursor_timer >= CURSOR_BLINK_INTERVAL) {
        cursor_timer -= CURSOR_BLINK_INTERVAL;
    }
    bool show_cursor = cursor_timer < CURSOR_BLINK_INTERVAL / 2.0f;

    // Update message timestamps
    for (int i = 0; i < console->line_count; i++) {
        console->timestamps[i] += FIXED_DT;
        if (console->timestamps[i] > MESSAGE_DURATION && !console->active) {
            // Shift messages up when expired
            for (int j = i; j < console->line_count - 1; j++) {
                my_strcpy(console->text[j], console->text[j + 1]);
                console->timestamps[j] = console->timestamps[j + 1];
            }
            console->text[console->line_count - 1][0] = '\0';
            console->timestamps[console->line_count - 1] = 0.0f;
            console->line_count--;
            i--;
        }
    }

    if (!console->active && console->line_count == 0) {
        return;
    }

    // Update console textures
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {128, 128, 128, 255}; // Gray for suggestion
    int max_text_w = 0, text_h = 0;
    int line_height = 0;
    TTF_SizeText(font, "A", NULL, &line_height); // Get approximate line height

    for (int i = 0; i < console->line_count; i++) {
        if (console->console_textures[i]) {
            SDL_DestroyTexture(console->console_textures[i]);
            console->console_textures[i] = NULL;
        }
        if (console->text[i][0] != '\0') {
            SDL_Surface* surface = TTF_RenderText_Solid(font, console->text[i], white);
            if (surface) {
                console->console_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
                int text_w;
                SDL_QueryTexture(console->console_textures[i], NULL, NULL, &text_w, &text_h);
                max_text_w = SDL_max(max_text_w, text_w);
                SDL_FreeSurface(surface);
            }
        }
    }

    // Update input texture with cursor
    if (console->input_texture) {
        SDL_DestroyTexture(console->input_texture);
        console->input_texture = NULL;
    }
    if (console->suggestion_texture) {
        SDL_DestroyTexture(console->suggestion_texture);
        console->suggestion_texture = NULL;
    }
    if (console->active) {
        char input_display[MAX_COMMAND_LENGTH + 4];
        if (show_cursor) {
            // Insert cursor at the correct position
            snprintf(input_display, sizeof(input_display), ">%.*s|%s", console->cursor_pos, console->input, console->input + console->cursor_pos);
        } else {
            snprintf(input_display, sizeof(input_display), ">%.*s %s", console->cursor_pos, console->input, console->input + console->cursor_pos);
        }
        SDL_Surface* input_surface = TTF_RenderText_Solid(font, input_display, white);
        if (input_surface) {
            console->input_texture = SDL_CreateTextureFromSurface(renderer, input_surface);
            int text_w;
            SDL_QueryTexture(console->input_texture, NULL, NULL, &text_w, &text_h);
            max_text_w = SDL_max(max_text_w, text_w);
            SDL_FreeSurface(input_surface);
        }

        // Update suggestion texture
        if (console->suggestion[0] != '\0') {
            SDL_Surface* suggestion_surface = TTF_RenderText_Solid(font, console->suggestion, gray);
            if (suggestion_surface) {
                console->suggestion_texture = SDL_CreateTextureFromSurface(renderer, suggestion_surface);
                int text_w;
                SDL_QueryTexture(console->suggestion_texture, NULL, NULL, &text_w, &text_h);
                max_text_w = SDL_max(max_text_w, text_w);
                SDL_FreeSurface(suggestion_surface);
            }
        }
    }

    int cam_h;
    if (SDL_GetRendererOutputSize(renderer, NULL, &cam_h) != 0) {
        printf("Error: SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
        cam_h = CAMERA_H;
    }

    // Calculate background size
    int margin = 5;
    int bg_width = max_text_w + 2 * margin;
    int bg_height = (console->line_count + (console->active ? 1 : 0)) * (text_h + 5) + 2 * margin;
    int bg_x = 10; // 10 pixels from left
    int bg_y = cam_h - 20 - text_h - margin; // 10 pixels from bottom, adjusted for input line
    if (console->active) {
        bg_y -= console->line_count * (text_h + 5); // Adjust for history lines
    } else {
        bg_y -= (console->line_count - 1) * (text_h + 5); // Only history lines
    }

    // Draw background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect console_rect = {bg_x, bg_y, (bg_width < 500) ? 500 : bg_width, bg_height};
    SDL_RenderFillRect(renderer, &console_rect);

    // Render console history (newest at bottom)
    for (int i = 0; i < console->line_count; i++) {
        if (console->console_textures[i]) {
            int text_w, text_h;
            SDL_QueryTexture(console->console_textures[i], NULL, NULL, &text_w, &text_h);
            SDL_Rect text_rect = {
                bg_x + margin,
                bg_y + margin + (console->line_count - 1 - i) * (text_h + 5), // Reverse order for newest at bottom
                text_w,
                text_h
            };
            SDL_RenderCopy(renderer, console->console_textures[i], NULL, &text_rect);
        }
    }

    // Render current input
    if (console->active && console->input_texture) {
        int text_w, text_h;
        SDL_QueryTexture(console->input_texture, NULL, NULL, &text_w, &text_h);
        SDL_Rect input_rect = {
            bg_x + margin,
            cam_h - 10 - text_h, // 10 pixels from bottom
            text_w,
            text_h
        };
        SDL_RenderCopy(renderer, console->input_texture, NULL, &input_rect);

        // Render suggestion text
        if (console->suggestion_texture) {
            int suggestion_w, suggestion_h;
            SDL_QueryTexture(console->suggestion_texture, NULL, NULL, &suggestion_w, &suggestion_h);
            SDL_Rect suggestion_rect = {
                bg_x + margin + text_w, // Position after input text
                cam_h - 10 - suggestion_h,
                suggestion_w,
                suggestion_h
            };
            SDL_RenderCopy(renderer, console->suggestion_texture, NULL, &suggestion_rect);
        }
    }
}

void free_console(Console* console) {
    for (int i = 0; i < MAX_CONSOLE_LINES; i++) {
        if (console->console_textures[i]) {
            SDL_DestroyTexture(console->console_textures[i]);
            console->console_textures[i] = NULL;
        }
    }
    if (console->input_texture) {
        SDL_DestroyTexture(console->input_texture);
        console->input_texture = NULL;
    }
    if (console->suggestion_texture) {
        SDL_DestroyTexture(console->suggestion_texture);
        console->suggestion_texture = NULL;
    }
}
