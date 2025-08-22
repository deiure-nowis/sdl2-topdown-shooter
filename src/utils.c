#include "utils.h"
#include "common.h"

float absf(float x){
	union{
		float f;
		uint32_t i;
	} u = {x};
	u.i &= 0x7FFFFFFF;
	return u.f;
}

int absi(int x){
	int mask = x >> 31;
	return (x + mask) ^ mask;
}

float maxf(float a, float b) {
    return (a > b) ? a : b;
}

float minf(float a, float b) {
    return (a < b) ? a : b;
}


float my_sqrt(float x){
	if(x<0.0) return 0.0 / 0.0;
	if(x==0 || x==1) return x;
	float result = (x>1) ? x*0.5 : x*2.0;
	float accuracy = 0.0001f;
	int maxf = 50;
	do{
		result = (result + x / result) / 2.0f;
		maxf--;
	} while(absf(result*result - x) > accuracy && maxf > 0);
	return result;
}

float my_atan2f(float y, float x){
	if(x==0.0f) return (y>0.0f) ? MA_PI / 2.0f : (y<0.0f) ? -MA_PI / 2.0f : 0.0f;
	float atan;
	float z = y / x;
	if(absf(z) < 1.0f){
		atan = z / (1.0f + 0.28f * z * z);
		if(x<0.0f) return (y<0.0f) ? atan - MA_PI : atan + MA_PI;
	}else{
		atan = MA_PI / 2.0f - z / (z * z + 0.28f);
		if(y<0.0f) return atan - MA_PI;
	}
	return atan;
}

float my_cosf(float x){
	while(x>MA_PI) x -= 2.0f * MA_PI;
	while(x<-MA_PI) x += 2.0f * MA_PI;
	float result = 1.0f;
	float term = 1.0f;
	float x2 = x * x;
	for(int i=1; i<=6; i++){
		term *= -x2 / ((2*i - 1)*(2*i));
		result += term;
	}
	return result;
}

float my_sinf(float x){
	while(x>MA_PI) x -= 2.0f * MA_PI;
	while(x<-MA_PI) x += 2.0f * MA_PI;
	float result = x;
	float term = x;
	float x2 = x * x;
	for(int i=1; i<=6; i++){
		term *= -x2 / ((2*i)*(2*i + 1));
		result += term;
	}
	return result;
}

SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer) {
	SDL_Surface* surface = IMG_Load(path);
	if (!surface) {
		printf("Failed to load image %s: %s\n", path, IMG_GetError());
		return NULL;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) {
		printf("Failed to create texture: %s\n", SDL_GetError());
	}
	return texture;
}

SDL_Texture* create_background_texture(SDL_Renderer* renderer, SDL_Texture* tile, int world_w, int world_h) {
	SDL_Texture* bg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, world_w, world_h);
	SDL_SetRenderTarget(renderer, bg);
	for (int y = 0; y < world_h; y += TILE_SIZE) {
		for (int x = 0; x < world_w; x += TILE_SIZE) {
			SDL_Rect dst = {x, y, TILE_SIZE, TILE_SIZE};
			SDL_RenderCopy(renderer, tile, NULL, &dst);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
	return bg;
}

void init_walls(World* world, SDL_Renderer* renderer, SDL_Texture* wall_texture_S, SDL_Texture* wall_texture_B, SDL_Texture* wall_texture_O) {
	int wall_count = 0;
	for (int y = 0; y < MAP_SIZE; y++)
		for (int x = 0; x < MAP_SIZE; x++)
			if (world->map[y][x] == WALL_SMALL || world->map[y][x] == WALL_BULLETPROOF || world->map[y][x] == WALL_OPAQUE)
				wall_count++;
	world->walls = (Wall*)malloc(wall_count * sizeof(Wall));
	world->wall_count = wall_count;

	uint8_t temp_map[MAP_SIZE][MAP_SIZE];
	memcpy(temp_map, world->map, sizeof(temp_map));
	for (int y = 0; y < MAP_SIZE; y++) {
		for (int x = 0; x < MAP_SIZE; x++) {
			if (world->map[y][x] == WALL_SMALL || world->map[y][x] == WALL_BULLETPROOF || world->map[y][x] == WALL_OPAQUE) {
				int directions[8][2] = {
					{0, 1}, {1, 0}, {0, -1}, {-1, 0},
					{1, 1}, {1, -1}, {-1, 1}, {-1, -1}
				};
				for (int i = 0; i < 8; i++) {
					int nx = x + directions[i][0];
					int ny = y + directions[i][1];
					if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE && world->map[ny][nx] == WALL_NONE) {
						temp_map[ny][nx] = WALL_PATHFINDING_BLOCK;
					}
				}
			}
		}
	}
	memcpy(world->map, temp_map, sizeof(temp_map));

	int index = 0;
	for (int y = 0; y < MAP_SIZE; y++) {
		for (int x = 0; x < MAP_SIZE; x++) {
			if (world->map[y][x] == WALL_SMALL || world->map[y][x] == WALL_BULLETPROOF || world->map[y][x] == WALL_OPAQUE) {
				world->walls[index].x = x * TILE_SIZE;
				world->walls[index].y = y * TILE_SIZE;
				world->walls[index].w = TILE_SIZE;
				world->walls[index].h = TILE_SIZE;
				world->walls[index].type = world->map[y][x];
				if (world->map[y][x] == WALL_SMALL) {
					world->walls[index].texture = wall_texture_S;
				} else if (world->map[y][x] == WALL_BULLETPROOF) {
					world->walls[index].texture = wall_texture_B;
				} else if (world->map[y][x] == WALL_OPAQUE) {
					world->walls[index].texture = wall_texture_O;
				}
				index++;
			}
		}
	}
}

bool check_collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool is_valid_node(int x, int y, World* world) {
	if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) return false;
	return world->map[y][x] == WALL_NONE;
}

bool has_line_of_sight(float start_x, float start_y, float end_x, float end_y, World* world, bool block_by_bulletproof, bool block_by_opaque) {
    // Handle null world or invalid coordinates
    if (!world || start_x < 0 || start_y < 0 || end_x < 0 || end_y < 0 || 
        start_x >= WORLD_W || start_y >= WORLD_H || end_x >= WORLD_W || end_y >= WORLD_H) {
        return false;
    }

    // If start and end points are the same, return true
    if (start_x == end_x && start_y == end_y) {
        return true;
    }

    // Convert pixel coordinates to tile coordinates
    int x0 = (int)(start_x / TILE_SIZE);
    int y0 = (int)(start_y / TILE_SIZE);
    int x1 = (int)(end_x / TILE_SIZE);
    int y1 = (int)(end_y / TILE_SIZE);

    // Bresenham's line algorithm
    int dx = absi(x1 - x0);
    int dy = absi(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // Check if current tile is within bounds
        if (x0 < 0 || x0 >= MAP_SIZE || y0 < 0 || y0 >= MAP_SIZE) {
            return false;
        }

        // Check if current tile blocks line of sight
        if ((block_by_bulletproof && world->map[y0][x0] == WALL_BULLETPROOF) ||
            (block_by_opaque && world->map[y0][x0] == WALL_OPAQUE)) {
            return false;
        }

        // Reached the end tile
        if (x0 == x1 && y0 == y1) {
            break;
        }

        // Move to next tile
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    return true;
}

void SDL_RenderFillPolygon(SDL_Renderer* renderer, const SDL_Point* points, int count) {
    if (!renderer || !points || count < 3) {
        return; // Need at least 3 points to form a polygon
    }

    // Find min and max y coordinates
    int min_y = points[0].y;
    int max_y = points[0].y;
    for (int i = 1; i < count; i++) {
        if (points[i].y < min_y) min_y = points[i].y;
        if (points[i].y > max_y) max_y = points[i].y;
    }

    // Iterate over each scanline
    for (int y = min_y; y <= max_y; y++) {
        // Collect intersections with the scanline
        int intersections[362]; // Max number of intersections (same as max points in render_fov)
        int intersection_count = 0;

        for (int i = 0; i < count; i++) {
            int j = (i + 1) % count; // Next point (wraps around to 0 for last edge)
            int y0 = points[i].y;
            int y1 = points[j].y;
            int x0 = points[i].x;
            int x1 = points[j].x;

            // Check if scanline y intersects the edge
            if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y)) {
                // Compute x-coordinate of intersection
                float t = (float)(y - y0) / (y1 - y0);
                int x = x0 + t * (x1 - x0);
                intersections[intersection_count++] = x;
            }
        }

        // Sort intersections by x-coordinate
        for (int i = 0; i < intersection_count - 1; i++) {
            for (int j = i + 1; j < intersection_count; j++) {
                if (intersections[i] > intersections[j]) {
                    int temp = intersections[i];
                    intersections[i] = intersections[j];
                    intersections[j] = temp;
                }
            }
        }

        // Draw horizontal lines between pairs of intersections
        for (int i = 0; i < intersection_count - 1; i += 2) {
            SDL_RenderDrawLine(renderer, intersections[i], y, intersections[i + 1], y);
        }
    }
}

float ray_aabb_intersect(float px, float py, float dx, float dy, float minx, float miny, float maxx, float maxy) {
    float inv_dx = (dx != 0.0f) ? 1.0f / dx : (dx > 0 ? 1e30f : -1e30f);
    float inv_dy = (dy != 0.0f) ? 1.0f / dy : (dy > 0 ? 1e30f : -1e30f);

    float tx1 = (minx - px) * inv_dx;
    float tx2 = (maxx - px) * inv_dx;
    float ty1 = (miny - py) * inv_dy;
    float ty2 = (maxy - py) * inv_dy;

    float tmin = maxf(minf(tx1, tx2), minf(ty1, ty2));
    float tmax = minf(maxf(tx1, tx2), maxf(ty1, ty2));

    if (tmax < tmin || tmin < 0.0f) return -1.0f;
    return tmin;
}

float get_visibility_distance(float x1, float y1, float dx, float dy, World* world) {
    float min_dist = 1e30f;  // Large initial value
    for (int i = 0; i < world->wall_count; i++) {
        Wall* w = &world->walls[i];
        if (w->type == WALL_OPAQUE) {  // These block sight, based on current logic
            float t = ray_aabb_intersect(x1, y1, dx, dy, w->x, w->y, w->x + w->w, w->y + w->h);
            if (t > 0.0f && t < min_dist) min_dist = t;
        }
    }
    return min_dist;
}

size_t my_strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

char *my_strcpy(char *dest, const char *src) {
    char *start = dest;
    while ((*dest++ = *src++));
    return start;
}

char *my_strcat(char *dest, const char *src) {
    char *start = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return start;
}

char *my_strncat(char *dest, const char *src, size_t n) {
    char *start = dest;
    while (*dest) dest++;
    while (n-- && *src) *dest++ = *src++;
    *dest = '\0';
    return start;
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int my_strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}
