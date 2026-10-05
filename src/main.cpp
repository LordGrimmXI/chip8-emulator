#include <iostream>
#include <SDL.h>
#include "chip8.h"

// Each CHIP-8 pixel becomes a 10x10 SDL pixel block
constexpr int pixelSize = 10;

void render(SDL_Renderer* renderer, const Chip8& chip8) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < Chip8::DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < Chip8::DISPLAY_WIDTH; ++x) {
            // CHIP-8 display is stored row-by-row:
            // row * width + column
            int index = y * Chip8::DISPLAY_WIDTH + x;

            if (chip8.display[index]) {
                SDL_Rect pixel {
                    x * pixelSize,
                    y * pixelSize,
                    pixelSize,
                    pixelSize
                };

                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    // Show everything we drew
    SDL_RenderPresent(renderer);
}

int keyToIndex(SDL_Keycode key) {
    switch (key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;

        default:
            return -1; // Not a valid CHIP-8 key
    }
}

int main(int argc, char* argv[]) {
    bool running = true;

    std::cout << "Program started\n";

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    std::cout << "SDL initialized\n";

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        Chip8::DISPLAY_WIDTH * pixelSize,
        Chip8::DISPLAY_HEIGHT * pixelSize,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    std::cout << "Window created\n";

    // Create renderer for the window
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Renderer created\n";

    Chip8 chip8;

    // Load the 0 glyph at the top-left corner
    chip8.V[0] = 0;
    chip8.V[1] = 0;
    chip8.I = Chip8::FONT_START;

    chip8.execute(0xD015);

    // Try drawing another 0 at a different position
    chip8.V[0] = 10;
    chip8.V[1] = 5;

    chip8.execute(0xD015);

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    chip8.draw_flag = true;   // force a redraw on the next loop pass
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }

                int index = keyToIndex(event.key.keysym.sym);

                if (index != -1) {
                    chip8.keys[index] = true;
                }
            }
            else if (event.type == SDL_KEYUP) {
                int index = keyToIndex(event.key.keysym.sym);

                if (index != -1) {
                    chip8.keys[index] = false;
                }
            }
        }

        if (chip8.draw_flag) {
            render(renderer, chip8);
            chip8.draw_flag = false;
        }

        SDL_Delay(16); // Roughly 60 FPS
    }

    // Renderer must be destroyed before the window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "Program finished\n";

    return 0;
}