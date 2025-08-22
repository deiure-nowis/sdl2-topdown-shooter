#ifndef TYPES_H
#define TYPES_H

#include "common.h"

// Represents a basic entity with position, size, and angle
typedef struct{
    float x, y;        // Position coordinates in pixels
    float w, h;        // Width and height in pixels
    float angle;       // Rotation angle in degrees
    int path[MAP_SIZE * MAP_SIZE]; // Array storing pathfinding nodes
    int path_length;   // Number of nodes in the current path
} Entity;

// Represents the player with position, size, velocity, angle, and texture
typedef struct{
    float x, y;        // Position coordinates in pixels
    float w, h;        // Width and height in pixels
    float vel_x, vel_y;// Velocity in pixels per second
    float angle;       // Rotation angle in degrees
    SDL_Texture* texture; // Texture for rendering the player
} Player;

// Represents a bullet with position, velocity, lifetime, activity status, and owner
typedef struct{
    float x, y;        // Position coordinates in pixels
    float vel_x, vel_y;// Velocity in pixels per second
    float lifetime;    // Time remaining before bullet expires (in seconds)
    bool active;       // Whether the bullet is active
    int owner;         // Owner of the bullet (0 for enemy, 1 for player)
} Bullet;

// Enumerates the possible states of an enemy
typedef enum{
    FREE = 0,          // Enemy is idle and not engaged
    SHOOT = 1,         // Enemy is shooting at the player
    TAKE_COVER = 2,    // Enemy is moving to or staying in cover
    CHASE = 3,         // Enemy is chasing the player
    SEARCHING = 4      // Enemy is searching for the player
} EnemyState;

// Represents an enemy with position, size, velocity, health, and AI-related properties
typedef struct{
    float x, y;        // Position coordinates in pixels
    float w, h;        // Width and height in pixels
    float vel_x, vel_y;// Velocity in pixels per second
    float hp;          // Health points
    float respawn_timer;// Time until respawn after death (in seconds)
    float path_timer;  // Timer for pathfinding updates
    bool active;       // Whether the enemy is active
    SDL_Texture* texture; // Texture for rendering the enemy
    int path[MAP_SIZE * MAP_SIZE]; // Array storing pathfinding nodes
    int path_length;   // Number of nodes in the current path
    bool force_path_recalc; // Flag to force path recalculation
    float angle;       // Rotation angle in degrees
    EnemyState state;  // Current state of the enemy
    float shoot_timer; // Timer for shooting cooldown
    float decision_timer;// Timer for AI decision-making
    bool in_cover;     // Whether the enemy is in cover
    int flag_id;       // ID of the associated flag for spawning
} Enemy;

// Enumerates the types of walls in the game
typedef enum{
    WALL_NONE = 0,     // Empty tile (no wall)
    WALL_SMALL = 1,    // Small wall (provides cover)
    WALL_BULLETPROOF = 2, // Bulletproof wall (blocks bullets)
    WALL_OPAQUE = 3,   // Opaque wall (blocks visibility and bullets)
    WALL_PATHFINDING_BLOCK = 4 // Tile blocked for pathfinding (adjacent to walls)
} WallType;

// Represents a wall with position, size, type, and texture
typedef struct{
    float x, y;        // Position coordinates in pixels
    float w, h;        // Width and height in pixels
    WallType type;     // Type of the wall
    SDL_Texture* texture; // Texture for rendering the wall
} Wall;

// Represents a flag (spawn point) with position, size, texture, and spawning properties
typedef struct{
    float x, y;        // Position coordinates in pixels
    float w, h;        // Width and height in pixels
    SDL_Texture* texture; // Texture for rendering the flag
    bool active;       // Whether the flag is active
    int enemy_count;   // Number of enemies to spawn at this flag
    float spawn_timer; // Timer for spawning enemies
    int enemies_spawned; // Number of enemies spawned so far
} Flag;

// Represents the game world with dimensions, background, map, walls, and flags
typedef struct{
    int w, h;          // World width and height in pixels
    SDL_Texture* background; // Texture for the world background
    SDL_Texture* minimap_texture; // Texture for the minimap (if pre-rendered)
    uint8_t map[MAP_SIZE][MAP_SIZE]; // 2D array representing the map grid
    Wall* walls;       // Array of walls
    int wall_count;    // Number of walls
    Flag* flags;       // Ascending
    int flag_count;   // Number of flags
    float fps;			// Game fps
} World;

// Represents the camera for rendering the game view
typedef struct{
    float x, y;        // Camera position in pixels
    int w, h;          // Camera width and height in pixels
} Camera;

// Represents a node in the pathfinding algorithm
typedef struct{
    int x, y;          // Coordinates of the node (in tiles)
    int g;             // Cost from start to this node
    int h;             // Estimated cost to goal (heuristic)
    int f;             // Total cost (g + h)
    int parent_x, parent_y; // Coordinates of the parent node
} Node;

// Represents the in-game console for command input and output
typedef struct {
    char text[MAX_CONSOLE_LINES][MAX_COMMAND_LENGTH]; // Array of console message lines
    float timestamps[MAX_CONSOLE_LINES]; // Timestamps for message expiration
    SDL_Texture* console_textures[MAX_CONSOLE_LINES]; // Textures for rendering console messages
    char input[MAX_COMMAND_LENGTH]; // Current input string
    SDL_Texture* input_texture; // Texture for rendering input text
    char suggestion[MAX_COMMAND_LENGTH]; // Autocomplete suggestion text
    SDL_Texture* suggestion_texture; // Texture for rendering suggestion text
    char history[MAX_HISTORY][MAX_COMMAND_LENGTH]; // Command history
    int history_count; // Number of commands in history
    int history_index; // Current position in history (-1 for current input)
    int line_count;    // Number of active console message lines
    int current_line;  // Current line for input or display
    int input_length;  // Length of the current input string
    int cursor_pos;    // Cursor position in the input string
    bool active;       // Whether the console is active
} Console;

// Represents the state of the game (toggles and settings)
typedef struct{
    bool minimap;      // Whether the minimap is visible
    bool is_fullscreen;// Whether the game is in fullscreen mode
    bool spawn_enabled;// Whether enemy spawning is enabled
} GameState;

typedef enum {
    MAIN_MENU = 0,
    OPTIONS_MENU = 1
} MenuType;

typedef struct {
    char text[MENU_OPTION_TEXT_LENGTH];
    SDL_Rect rect;
    SDL_Texture* text_texture;
    bool is_hovered;
} MenuOption;

typedef struct {
    bool active;
    MenuType type;
    MenuOption main_options[MENU_OPTION_COUNT];
    MenuOption options_menu_options[MENU_OPTION_COUNT];
    int selected_option; // -1 for none
    GameState* game_state; // Pointer to game state for toggling
    TTF_Font* font; // Font for rendering text
} Menu;

#endif
