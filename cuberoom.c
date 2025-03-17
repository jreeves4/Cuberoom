#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAP_WIDTH 12
#define MAP_HEIGHT 12
#define MOVE_SPEED 0.1f
#define ROTATE_SPEED 3.0f
#define TEXTURE_WIDTH 32
#define TEXTURE_HEIGHT 32
#define TILE_SCALE 0.5f

#include "bricks.c"

int map[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

typedef struct {
    float x, y;
    float angle;
} Player;

SDL_Texture* loadTextureFromData(SDL_Renderer* renderer, const uint32_t* data, int width, int height) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)data, width, height, 32, width * 4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        printf("Unable to create surface from data: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void drawScene(SDL_Renderer* renderer, Player player, SDL_Texture* brickTexture) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Gray floor
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255); // Blue ceiling
    SDL_Rect ceiling = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &ceiling);

    float fov = 60.0f;
    float halfFov = fov / 2.0f;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float rayAngle = player.angle - halfFov + (fov * x / (float)SCREEN_WIDTH);
        float rad = rayAngle * M_PI / 180.0f;

        float rayX = player.x;
        float rayY = player.y;
        float distToWall = 0.0f;
        bool hitWall = false;
        float hitX = 0.0f;
        bool isVertical = false;

        float stepX = cosf(rad) * 0.05f;
        float stepY = sinf(rad) * 0.05f;

        while (!hitWall && distToWall < 20.0f) {
            rayX += stepX;
            rayY += stepY;
            distToWall = sqrtf((rayX - player.x) * (rayX - player.x) + (rayY - player.y) * (rayY - player.y));

            int mapX = (int)rayX;
            int mapY = (int)rayY;

            if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                break;
            }

            if (map[mapX][mapY] == 1) {
                hitWall = true;
                float testX = rayX - stepX / 2.0f;
                float testY = rayY - stepY / 2.0f;
                if ((int)testX != (int)rayX) {
                    isVertical = true;
                    hitX = rayY;
                } else {
                    isVertical = false;
                    hitX = rayX;
                }
            }
        }

        if (hitWall) {
            if (distToWall < 0.1f) distToWall = 0.1f; // Still prevent division by zero

            int wallHeight = (int)(SCREEN_HEIGHT / distToWall);
            if (wallHeight > SCREEN_HEIGHT) wallHeight = SCREEN_HEIGHT;

            float texX = fmodf(hitX / TILE_SCALE, 1.0f) * TEXTURE_WIDTH;
            if (texX < 0) texX += TEXTURE_WIDTH;
            int texXInt = (int)texX % TEXTURE_WIDTH;

            float texHeightScale = (float)wallHeight / (TEXTURE_HEIGHT / TILE_SCALE);
            SDL_Rect srcRect = {texXInt, 0, 1, TEXTURE_HEIGHT};
            SDL_Rect destRect = {x, (SCREEN_HEIGHT - wallHeight) / 2, 1, wallHeight};

            SDL_SetTextureScaleMode(brickTexture, SDL_ScaleModeNearest);
            SDL_RenderCopy(renderer, brickTexture, &srcRect, &destRect);
        }
    }

    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Cuberoom - 3D Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* brickTexture = loadTextureFromData(renderer, bricks_data[0], TEXTURE_WIDTH, TEXTURE_HEIGHT);
    if (!brickTexture) {
        printf("Failed to load brick texture\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player = {2.0f, 2.0f, 90.0f}; // Start in open space
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float dirX = cosf(player.angle * M_PI / 180.0f);
        float dirY = sinf(player.angle * M_PI / 180.0f);
        float perpX = -dirY;
        float perpY = dirX;

        if (keys[SDL_SCANCODE_W]) {
            float nextX = player.x + dirX * MOVE_SPEED;
            float nextY = player.y + dirY * MOVE_SPEED;
            if (map[(int)nextX][(int)player.y] == 0) player.x = nextX;
            if (map[(int)player.x][(int)nextY] == 0) player.y = nextY;
        }
        if (keys[SDL_SCANCODE_S]) {
            float nextX = player.x - dirX * MOVE_SPEED;
            float nextY = player.y - dirY * MOVE_SPEED;
            if (map[(int)nextX][(int)player.y] == 0) player.x = nextX;
            if (map[(int)player.x][(int)nextY] == 0) player.y = nextY;
        }
        if (keys[SDL_SCANCODE_D]) {
            float nextX = player.x + perpX * MOVE_SPEED;
            float nextY = player.y + perpY * MOVE_SPEED;
            if (map[(int)nextX][(int)player.y] == 0) player.x = nextX;
            if (map[(int)player.x][(int)nextY] == 0) player.y = nextY;
        }
        if (keys[SDL_SCANCODE_A]) {
            float nextX = player.x - perpX * MOVE_SPEED;
            float nextY = player.y - perpY * MOVE_SPEED;
            if (map[(int)nextX][(int)player.y] == 0) player.x = nextX;
            if (map[(int)player.x][(int)nextY] == 0) player.y = nextY;
        }
        if (keys[SDL_SCANCODE_LEFT]) {
            player.angle -= ROTATE_SPEED;
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            player.angle += ROTATE_SPEED;
        }

        drawScene(renderer, player, brickTexture);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(brickTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}