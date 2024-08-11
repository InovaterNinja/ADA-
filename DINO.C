#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

// Constants for the game
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 400;
const int DINO_WIDTH = 50;
const int DINO_HEIGHT = 50;
const int OBSTACLE_WIDTH = 20;
const int OBSTACLE_HEIGHT = 50;
const int LINE_HEIGHT = 10;
const int GRAVITY = 1;
const int GAME_OVER_TEXT_SIZE = 36;
const int SCORE_TEXT_SIZE = 24;
const int SCORE_INCREMENT = 10;

// Variables for jump dynamics
const int JUMP_STRENGTH = 20;
const int JUMP_VELOCITY = -20;
const int FALL_VELOCITY = 15;

// Function to render "GAME OVER" text
void renderGameOver(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY) {
    SDL_Color color = {255, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "GAME OVER better luck next life", color);
    if (surface == NULL) {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect textRect = {startX, startY, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Function to render the score
void renderScore(SDL_Renderer *renderer, TTF_Font *font, int score) {
    SDL_Color color = {255, 255, 255, 255}; // White color
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);

    SDL_Surface *surface = TTF_RenderText_Solid(font, scoreText, color);
    if (surface == NULL) {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect textRect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Function to reset game state
void resetGame(int *dinoY, int *jumpVelocity, int *obstacleX, bool *gameover, int *score, int *obstacleSpeed) {
    *dinoY = GROUND_HEIGHT - DINO_HEIGHT;
    *jumpVelocity = 0;
    *obstacleX = WINDOW_WIDTH;
    *gameover = false;
    *score = 0;
    *obstacleSpeed = 5; // Reset obstacle speed
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("JUMPING PLANE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load Dino image as SDL_Texture
    SDL_Surface *dinoSurface = IMG_Load("/mnt/c/Users/abdul/Desktop/dino.png");
    if (!dinoSurface) {
        printf("Failed to load dino image! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture *dinoTexture = SDL_CreateTextureFromSurface(renderer, dinoSurface);
    SDL_FreeSurface(dinoSurface);
    if (!dinoTexture) {
        printf("Failed to create texture from dino image! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load background image as SDL_Texture
    SDL_Surface *bgSurface = IMG_Load("/mnt/c/Users/abdul/Desktop/background.png");
    if (!bgSurface) {
        printf("Failed to load background image! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(dinoTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture *bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    if (!bgTexture) {
        printf("Failed to create texture from background image! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(dinoTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load obstacle image as SDL_Texture
    SDL_Surface *obstacleSurface = IMG_Load("/mnt/c/Users/abdul/Desktop/obstacle.png");
    if (!obstacleSurface) {
        printf("Failed to load obstacle image! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(bgTexture);
        SDL_DestroyTexture(dinoTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture *obstacleTexture = SDL_CreateTextureFromSurface(renderer, obstacleSurface);
    SDL_FreeSurface(obstacleSurface);
    if (!obstacleTexture) {
        printf("Failed to create texture from obstacle image! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(bgTexture);
        SDL_DestroyTexture(dinoTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    bool quit = false;
    SDL_Event e;
    bool jumping = false;
    int dinoY = GROUND_HEIGHT - DINO_HEIGHT;
    int jumpVelocity = 0;
    int obstacleX = WINDOW_WIDTH;
    bool gameover = false;
    int score = 0;
    int obstacleSpeed = 5; // Initial obstacle speed

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE && !jumping && !gameover) {
                    jumping = true;
                    jumpVelocity = JUMP_VELOCITY;
                } else if (e.key.keysym.sym == SDLK_r && gameover) {
                    resetGame(&dinoY, &jumpVelocity, &obstacleX, &gameover, &score, &obstacleSpeed);
                }
            }
        }

        if (!gameover) {
            if (jumping) {
                dinoY += jumpVelocity;
                jumpVelocity += GRAVITY;

                if (dinoY >= GROUND_HEIGHT - DINO_HEIGHT) {
                    dinoY = GROUND_HEIGHT - DINO_HEIGHT;
                    jumping = false;
                }
            }

            obstacleX -= obstacleSpeed;
            if (obstacleX < -OBSTACLE_WIDTH) {
                obstacleX = WINDOW_WIDTH;
                score += SCORE_INCREMENT;
                // Increase speed every 50 points
                if (score % 50 == 0) {
                    obstacleSpeed += 2; // Increase the obstacle speed
                }
            }

            if (obstacleX <= 100 + DINO_WIDTH && obstacleX + OBSTACLE_WIDTH >= 100 && GROUND_HEIGHT - OBSTACLE_HEIGHT <= dinoY + DINO_HEIGHT) {
                gameover = true;
            }
        }

        SDL_RenderClear(renderer);

        // Render background texture
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect lineRect = {0, GROUND_HEIGHT, WINDOW_WIDTH, LINE_HEIGHT};
        SDL_RenderFillRect(renderer, &lineRect);

        // Render Dino texture
        SDL_Rect dinoRect = {100, dinoY, DINO_WIDTH, DINO_HEIGHT};
        SDL_RenderCopy(renderer, dinoTexture, NULL, &dinoRect);

        // Render obstacle texture
        SDL_Rect obstacleRect = {obstacleX, GROUND_HEIGHT - OBSTACLE_HEIGHT, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
        SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacleRect);

        renderScore(renderer, font, score);

        if (gameover) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            int startX = (WINDOW_WIDTH - GAME_OVER_TEXT_SIZE * 9) / 2;
            int startY = (WINDOW_HEIGHT - GAME_OVER_TEXT_SIZE) / 2;

            renderGameOver(renderer, font, startX, startY);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(20);
    }

    // Cleanup
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(dinoTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
