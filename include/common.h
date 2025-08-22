#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
//#include <math.h>

// Defines the world width in pixels (2048 pixels)
#define WORLD_W 2048
// Defines the world height in pixels (2048 pixels)
#define WORLD_H 2048
// Defines the size of the map grid (64x64 tiles)
#define MAP_SIZE 64
// Defines the size of each tile in pixels (32x32 pixels)
#define TILE_SIZE 32
// Defines the camera width in pixels, scaled by CAM_SCALE (500 * 2 = 1000 pixels)
#define CAMERA_W (500 * CAM_SCALE)
// Defines the camera height in pixels, scaled by CAM_SCALE (300 * 2 = 600 pixels)
#define CAMERA_H (300 * CAM_SCALE)
// Defines the camera scaling factor (2x for resolution scaling)
#define CAM_SCALE 2

// Defines the maximum field of view range for visibility calculations (700 pixels)
#define FOV_RANGE 700.0f
// Defines the radius of the circular FOV around the player (64 pixels)
#define FOV_CIRCLE_R 64.0f
// Defines half of the field of view angle in degrees (45 degrees, so full FOV is 90 degrees)
#define FOV_HALF_ANGLE 45.0f
// Defines the transitional range for fading visibility in FOV (650 pixels)
#define FOV_TRANSITIONAL_RANGE 650.0f
#define FOV_CIRCLE_TRANSITIONAL_RANGE (FOV_CIRCLE_R - 10.0f)
// Defines the alpha value for grayed-out areas in FOV (150 for partial transparency)
#define FOV_GRAY_ALPHA 150
// Defines the player's rotation speed in degrees per second (270 degrees/second for a 180-degree turn in 0.66 seconds)
#define PLAYER_ROTATION_SPEED 270.0f
// Defines the maximum rotation per fixed update for the player (based on rotation speed and fixed delta time)
#define MAX_PLAYER_ROTATION (PLAYER_ROTATION_SPEED * FIXED_DT * 1.0f)
// Number of rays for smooth circle approximation (higher = smoother, but 360 is ample for 90-degree FOV)
#define FOV_RAY_COUNT 64

// Defines the maximum number of bullets that can exist at once (200 bullets)
#define MAX_BULLETS 200
// Defines the speed of bullets in pixels per second (1024 pixels/second)
#define BULLET_SPEED 1024.0f
// Defines the lifetime of bullets in seconds (0.5 seconds)
#define BULLET_LIFETIME 0.5f

// Defines the maximum number of enemies that can exist at once (400 enemies)
#define MAX_ENEMIES 400
// Defines the default health points for enemies (50 HP)
#define ENEMY_HP 50.0f
// Defines the time before an enemy respawns after being defeated (10 seconds)
#define ENEMY_RESPAWN_DELAY 10.0f
// Defines the movement speed of enemies in pixels per second (200 pixels/second)
#define ENEMY_SPEED 200.0f
// Defines the enemy's rotation speed in degrees per second (240 degrees/second for a 180-degree turn in 0.75 seconds)
#define ENEMY_ROTATION_SPEED 240.0f
// Defines half of the enemy's field of view angle in degrees (64 degrees, so full FOV is 128 degrees)
#define ENEMY_HALF_ANGLE 64

// Defines the maximum range for pathfinding calculations in pixels (600 pixels)
#define PATHFINDING_RANGE 600.0f
// Defines the interval between pathfinding updates in seconds (0.5 seconds)
#define PATHFINDING_INTERVAL 0.5f
// Defines the maximum number of iterations for the pathfinding algorithm (1000 iterations)
#define MAX_PATHFINDING_ITERATIONS 1000
// Defines the time threshold to detect if an enemy is stuck in seconds (1 second)
#define STUCK_THRESHOLD 1.0f

// Defines the maximum distance for enemies to seek cover in pixels (544 pixels)
#define MAX_COVER_DISTANCE 544.0f
// Defines the range within which enemies can shoot in pixels (384 pixels)
#define SHOOTING_RANGE 384.0f
// Defines the cooldown period between enemy shots in seconds (0.5 seconds)
#define SHOOT_COOLDOWN 0.5f
// Defines the interval for enemy decision-making in seconds (1 second)
#define DECISION_INTERVAL 1.0f

// Defines the fixed time step for physics updates in seconds (1/60 seconds for 60 Hz updates)
#define FIXED_DT (1.0f / 60.0f)
// Defines the interval for updating the FPS display in seconds (1 second)
#define FPS_UPDATE_INTERVAL 1.0f
// Defines the target frames per second for the game (144 FPS)
#define TARGET_FPS 144.0f
// Defines the target frame time in milliseconds (1000/144 ≈ 6.944 ms)
#define TARGET_FRAME_TIME (1000.0f / TARGET_FPS)

// Defines the maximum length of a console command in characters (256 characters)
#define MAX_COMMAND_LENGTH 256
// Defines the maximum number of console message lines displayed (10 lines)
#define MAX_CONSOLE_LINES 10
// Defines the duration that console messages remain visible in seconds (30 seconds)
#define MESSAGE_DURATION 30.0f
// Defines the maximum number of commands stored in the console history (50 commands)
#define MAX_HISTORY 50
// Defines the number of available console commands (2 commands)
#define NUM_COMMANDS 2
// Defines the interval for the console cursor blink cycle in seconds (2 seconds)
#define CURSOR_BLINK_INTERVAL 2.0f

#define MA_PI (3.1415926535897932846f)

extern 	SDL_Texture* fov_mask;

#endif
