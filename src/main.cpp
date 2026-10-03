#include <iostream>
#include <SDL.h>
#include "chip8.h"

void render(SDL_Renderer* renderer, const Chip8& chip8) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Each CHIP-8 pixel becomes a 10x10 SDL pixel block
    const int pixelSize = 10;

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

int main(int argc, char* argv[]) {
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
        Chip8::DISPLAY_WIDTH * 10,
        Chip8::DISPLAY_HEIGHT * 10,
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

    // Render the current display
    render(renderer, chip8);

    // Keep the window visible for 5 seconds
    SDL_Delay(5000);

    // Renderer must be destroyed before the window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "Program finished\n";

    return 0;
}