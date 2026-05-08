#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

// Platform: thin SDL2 wrapper so the PPU/CPU stay free of any SDL include.
// For now: create a window + renderer and draw a buffer of 1-byte-per-pixel
// indices (0-3) as grayscale. No input handling beyond window-close.

struct SDL_Window;
struct SDL_Renderer;

class Platform {
public:
    Platform(const char* title, int textureW, int textureH, int scale);
    ~Platform();

    // `buffer` points to textureW * textureH bytes, each 0-3.
    // Mapped to 4 grayscale shades.
    void render(const uint8_t* buffer);

    // Pump SDL events. Returns false if the user closed the window.
    bool processInput();

private:
    int width;
    int height;
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
};