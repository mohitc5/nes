#include "platform.h"

Platform::Platform(const char* title, int textureW, int textureH, int scale)
    : width(textureW), height(textureH) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(textureW * scale, textureH * scale, 0, &window, &renderer);
    SDL_SetWindowTitle(window, title);
    SDL_RenderSetScale(renderer, (float)scale, (float)scale);
}

Platform::~Platform() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::render(const uint8_t* buffer) {
    static const uint8_t shades[4] = {0, 85, 170, 255};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t v = buffer[y * width + x] & 0x3;
            uint8_t s = shades[v];
            SDL_SetRenderDrawColor(renderer, s, s, s, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_RenderPresent(renderer);
}

bool Platform::processInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return false;
    }
    return true;
}