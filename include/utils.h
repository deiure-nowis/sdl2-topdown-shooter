#ifndef UTILS_H
#define UTILS_H

#include "types.h"

float absf(float x);
int absi(int x);
float my_sqrt(float x);
float my_atan2f(float y,float x);
float my_cosf(float x);
float my_sinf(float x);
SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer);
SDL_Texture* create_background_texture(SDL_Renderer* renderer, SDL_Texture* tile, int world_w, int world_h);
void init_walls(World* world, SDL_Renderer* renderer, SDL_Texture* wall_texture_S, SDL_Texture* wall_texture_B, SDL_Texture* wall_texture_O);
bool check_collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
bool is_valid_node(int x, int y, World* world);
bool has_line_of_sight(float start_x, float start_y, float end_x, float end_y, World* world, bool block_by_bulletproof, bool block_by_opaque);
void SDL_RenderFillPolygon(SDL_Renderer* renderer, const SDL_Point* points, int count);

size_t my_strlen(const char *str);
char *my_strcpy(char *dest, const char *src);
char *my_strcat(char *dest, const char *src);
char *my_strncat(char *dest, const char *src, size_t n);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, size_t n);

#endif
