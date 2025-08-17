#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 64
#define OPEN 0
#define SOLID 1
#define BREAKABLE 2
#define LOCKED 3

// Directions for maze generation (right, down, left, up)
int dx[] = {1, 0, -1, 0};
int dy[] = {0, 1, 0, -1};

// Maze array
int maze[SIZE][SIZE];

// Function to initialize maze with solid walls
void init_maze() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            maze[i][j] = SOLID; // All cells start as solid walls
        }
    }
}

// Function to check if a cell is valid
int is_valid(int x, int y) {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

// Function to generate maze with 2-node-wide paths
void generate_maze(int x, int y) {
    maze[x][y] = OPEN; // Mark current cell as open path
    if (is_valid(x + 1, y)) maze[x + 1][y] = OPEN; // Make path 2 nodes wide (horizontal)
    if (is_valid(x, y + 1)) maze[x][y + 1] = OPEN; // Make path 2 nodes wide (vertical)

    // Randomize direction order
    int directions[4] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = directions[i];
        directions[i] = directions[j];
        directions[j] = temp;
    }

    // Try each direction
    for (int i = 0; i < 4; i++) {
        int dir = directions[i];
        int nx = x + dx[dir] * 4; // Move 4 cells to account for 2-cell path + wall
        int ny = y + dy[dir] * 4;

        if (is_valid(nx, ny) && maze[nx][ny] == SOLID) {
            // Create 2-node-wide path with 80% probability
            if (rand() % 100 < 80) {
                // Carve 2-node-wide path
                maze[x + dx[dir]][y + dy[dir]] = OPEN;
                maze[x + dx[dir] * 2][y + dy[dir] * 2] = OPEN;
                if (dx[dir] != 0) { // Horizontal movement
                    maze[x + dx[dir]][y + 1] = OPEN;
                    maze[x + dx[dir] * 2][y + 1] = OPEN;
                } else { // Vertical movement
                    maze[x + 1][y + dy[dir]] = OPEN;
                    maze[x + 1][y + dy[dir] * 2] = OPEN;
                }
                generate_maze(nx, ny);
            }
        }
    }
}

// Function to create empty space in the center
void create_center_space() {
    // Create a 10x10 empty space from (27,27) to (36,36)
    for (int i = 27; i <= 36; i++) {
        for (int j = 27; j <= 36; j++) {
            maze[i][j] = OPEN;
        }
    }
}

// Function to assign random wall types
void assign_wall_types() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (maze[i][j] == SOLID) {
                int r = rand() % 3;
                maze[i][j] = (r == 0) ? SOLID : (r == 1) ? BREAKABLE : LOCKED;
            }
        }
    }
}

// Function to write maze to file
void write_maze_to_file() {
    FILE *fp = fopen("maze.txt", "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    for (int i = 0; i < SIZE; i++) {
        fprintf(fp, "{");
        for (int j = 0; j < SIZE; j++) {
            fprintf(fp, "%d", maze[i][j]);
            if (j < SIZE - 1) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "},\n");
    }

    fclose(fp);
}

int main() {
    srand(time(NULL)); // Seed random number generator
    init_maze();
    generate_maze(0, 0); // Start maze generation from (0,0)
    create_center_space(); // Add empty space in the center
    assign_wall_types();
    // Start (0,0) and end (63,63) are already 0 (OPEN) from maze generation
    write_maze_to_file();
    return 0;
}
