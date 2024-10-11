//
// Created by Brian Bonafilia on 10/6/24.
//

#include <SDL2/SDL.h>
#include <iostream>
#include <cstdint>
#include <random>
#include "cpu.h"

namespace GUI {

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *game_pixels;

const int kScreenWidth = 800;
const int kSreenHeight = 720;

const int kPixelWidth = 160;
const int kPixelHeight = 144;

void UpdateTexture() {
  auto* pixels = new uint32_t[160*144];
  for (int i = 0; i < 160 * 144; i++) {
    pixels[i] = random() % 0xFFFFFFFF;
  }
  SDL_UpdateTexture(game_pixels, nullptr, pixels,kPixelWidth * sizeof(uint32_t));
  delete[] pixels;
}

void Init(bool debug) {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    std::cerr << "Failed to initialize video" << std::endl;
    exit(1);
  }

  window = SDL_CreateWindow("GB EMU",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            kScreenWidth,
                            kSreenHeight,
                            SDL_WINDOW_OPENGL);
  if (window == nullptr) {
    std::cerr << "Failed to create window" << std::endl;
    exit(2);
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    std::cerr << "Failed to create renderer";
  }

  SDL_RenderSetLogicalSize(renderer, kPixelWidth, kPixelHeight);
  game_pixels =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, kPixelHeight, kPixelHeight);


  bool is_running = true;

  UpdateTexture();
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, game_pixels, nullptr, nullptr);
  SDL_RenderPresent(renderer);
  while (is_running) {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        is_running = false;
      }
    }
    CPU::ProcessInstruction(debug);
  }

  SDL_DestroyTexture(game_pixels);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

}



