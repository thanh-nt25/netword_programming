#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <unistd.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BULLET_SPEED 5
#define TANK_SPEED 3
#define MAX_BULLETS 10
#define TANK_ROTATE_SPEED 0.05f  // Rotation speed in radians

typedef struct {
    int x, y;
    int dx, dy;
    int bounces;
} Bullet;

typedef struct {
    int x, y;
    float angle;  // Tank's orientation in radians
} Tank;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* tankTexture = NULL;

int initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    window = SDL_CreateWindow("Tank Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    // Initialize SDL_image for loading PNG/JPG files
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }
    return 0;
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return newTexture;
}

void handleInput(Tank* tank, Bullet* bullets, int* bullet_count, int* running) {
    int spacebarPressed = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = 0;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    // Move forward in the direction the tank is facing
                    tank->x += cos(tank->angle) * TANK_SPEED;
                    tank->y += sin(tank->angle) * TANK_SPEED;
                    break;
                case SDLK_s:
                    // Move backward in the opposite direction
                    tank->x -= cos(tank->angle) * TANK_SPEED;
                    tank->y -= sin(tank->angle) * TANK_SPEED;
                    break;
                case SDLK_a:
                    tank->angle -= TANK_ROTATE_SPEED;  // Rotate counterclockwise
                    break;
                case SDLK_d:
                    tank->angle += TANK_ROTATE_SPEED;  // Rotate clockwise
                    break;
                case SDLK_SPACE:
                    if (!spacebarPressed && *bullet_count < MAX_BULLETS) {  // Shoot once per spacebar press
                        bullets[*bullet_count].x = tank->x + cos(tank->angle) * 25;  // Bullet fired from front of tank
                        bullets[*bullet_count].y = tank->y + sin(tank->angle) * 25;
                        bullets[*bullet_count].dx = cos(tank->angle) * BULLET_SPEED;
                        bullets[*bullet_count].dy = sin(tank->angle) * BULLET_SPEED;
                        bullets[*bullet_count].bounces = 0;
                        (*bullet_count)++;
                        spacebarPressed = 1;  // Set flag to prevent multiple bullets while holding space
                    }
                break;
            }
        }
        if (e.type == SDL_KEYUP) {
            if (e.key.keysym.sym == SDLK_SPACE) {
                spacebarPressed = 0;  // Reset flag when spacebar is released
            }
        }
    }
}

void update(Tank* tank, Bullet* bullets, int* bullet_count) {
    // Update bullet positions and check for collisions
    for (int i = 0; i < *bullet_count; i++) {
        bullets[i].x += bullets[i].dx;
        bullets[i].y += bullets[i].dy;

        // Bullet bouncing logic (bounce up to 3 times)
        if ((bullets[i].x <= 0 || bullets[i].x >= SCREEN_WIDTH) && bullets[i].bounces < 3) {
            bullets[i].dx = -bullets[i].dx;
            bullets[i].bounces++;
        }
        if ((bullets[i].y <= 0 || bullets[i].y >= SCREEN_HEIGHT) && bullets[i].bounces < 3) {
            bullets[i].dy = -bullets[i].dy;
            bullets[i].bounces++;
        }
    }
}

void render(Tank* tank, Bullet* bullets, int bullet_count) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Black background
    SDL_RenderClear(renderer);

    // Draw walls (simple borders of the screen)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, 0, 0, SCREEN_WIDTH, 0);   // Top wall
    SDL_RenderDrawLine(renderer, 0, 0, 0, SCREEN_HEIGHT);  // Left wall
    SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);  // Bottom wall
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);  // Right wall

    // Draw target
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect target = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50};
    SDL_RenderFillRect(renderer, &target);

    // Draw bullets
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < bullet_count; i++) {
        SDL_Rect bullet_rect = {bullets[i].x, bullets[i].y, 10, 10};
        SDL_RenderFillRect(renderer, &bullet_rect);
    }

    // Render the tank with rotation
    SDL_Rect tankRect = { (int)tank->x, (int)tank->y, 32, 32 }; // Tank image is 32x32
    SDL_RenderCopyEx(renderer, tankTexture, NULL, &tankRect, tank->angle * 180 / M_PI, NULL, SDL_FLIP_NONE);

    // Present the renderer
    SDL_RenderPresent(renderer);
}

int main() {
    int running = 1;
    Tank player_tank = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f};  // Initial tank position and angle
    Bullet bullets[MAX_BULLETS];
    int bullet_count = 0;

    // Networking setup
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    if (initSDL() != 0) {
        return -1;
    }

    tankTexture = loadTexture("tank.jpg");  // Load your tank image
    if (tankTexture == NULL) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    while (running) {
        handleInput(&player_tank, bullets, &bullet_count, &running);
        update(&player_tank, bullets, &bullet_count);
        render(&player_tank, bullets, bullet_count);
        SDL_Delay(16);  // Delay to cap framerate at ~60fps
    }

    close(sock);
    SDL_DestroyTexture(tankTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

