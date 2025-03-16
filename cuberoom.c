#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MOVE_SPEED 0.05f
#define ROTATE_SPEED 1.5f

// Simple 8x8 map (1 = wall, 0 = empty)
int map[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

typedef struct {
    float x, y;       // Player position
    float angle;      // View direction (degrees)
} Player;

void drawScene(SDL_Renderer* renderer, Player player) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    float fov = 60.0f;  // Field of view
    float halfFov = fov / 2.0f;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate the ray's angle based on the player's view angle
        float rayAngle = player.angle - halfFov + (fov * x / SCREEN_WIDTH);
        float rad = rayAngle * 3.14159f / 180.0f;

        // Raycasting setup
        float rayX = player.x;
        float rayY = player.y;
        float deltaX = cos(rad);
        float deltaY = sin(rad);
        float distance = 0.0f;
        int hit = 0;

        // Cast rays until hitting a wall or going out of bounds
        while (!hit && distance < 20.0f) { // Max distance to avoid infinite loops
            rayX += deltaX * 0.1f;
            rayY += deltaY * 0.1f;
            distance += 0.1f;

            int mapX = (int)rayX;
            int mapY = (int)rayY;

            if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                hit = 1; // Out of bounds
            } else if (map[mapX][mapY] == 1) {
                hit = 1; // Hit a wall
            }
        }

        // Remove the fisheye effect: Use the raw distance to the wall
        // No correction here: Just use the raw distance for wall height
        int wallHeight = (int)(SCREEN_HEIGHT / distance);
        if (wallHeight > SCREEN_HEIGHT) wallHeight = SCREEN_HEIGHT;

        // Draw the wall with the calculated height
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); // Light gray walls
        SDL_RenderDrawLine(renderer, x, (SCREEN_HEIGHT - wallHeight) / 2, 
                          x, (SCREEN_HEIGHT + wallHeight) / 2);

        // Floor and ceiling rendering
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray floor
        SDL_RenderDrawLine(renderer, x, SCREEN_HEIGHT / 2 + wallHeight / 2, x, SCREEN_HEIGHT);
        SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255); // Blue ceiling
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT / 2 - wallHeight / 2);
    }
}



int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Doom-like Room",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SCREEN_WIDTH, SCREEN_HEIGHT,
                                        SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    Player player = {2.0f, 2.0f, 0.0f}; // Start inside room
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float rad = player.angle * 3.14159265358979323f / 180.0f;

        // Movement
        float newX = player.x;
        float newY = player.y;

        if (keys[SDL_SCANCODE_W]) { // Forward
            newX += cos(rad) * MOVE_SPEED;
            newY += sin(rad) * MOVE_SPEED;
        }
        if (keys[SDL_SCANCODE_S]) { // Backward
            newX -= cos(rad) * MOVE_SPEED;
            newY -= sin(rad) * MOVE_SPEED;
        }
        if (keys[SDL_SCANCODE_A]) { // Strafe left
            newX += sin(rad) * MOVE_SPEED;
            newY -= cos(rad) * MOVE_SPEED;
        }
        if (keys[SDL_SCANCODE_D]) { // Strafe right
            newX -= sin(rad) * MOVE_SPEED;
            newY += cos(rad) * MOVE_SPEED;
        }
        if (keys[SDL_SCANCODE_RIGHT]) { // Rotate left
            player.angle += ROTATE_SPEED;
            if (player.angle >= 360) player.angle -= 360;
        }
        if (keys[SDL_SCANCODE_LEFT]) { // Rotate right
            player.angle -= ROTATE_SPEED;
            if (player.angle < 0) player.angle += 360;
        }

        // Collision check
        if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT) {
            if (map[(int)newX][(int)newY] == 0) {
                player.x = newX;
                player.y = newY;
            }
        }

        drawScene(renderer, player);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}