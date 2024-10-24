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

constexpr int kScreenWidth = 800;
constexpr int kSreenHeight = 720;

constexpr int kPixelWidth = 160;
constexpr int kPixelHeight = 144;

// May make this more accurate i guess lately, probably does not matter. Currently 60FPS.
constexpr uint32_t kFrameTimeInMs = 16;

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
    uint32_t startTime = SDL_GetTicks();
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        is_running = false;
      }
    }
    CPU::RunFrame(debug);
    uint32_t latency = SDL_GetTicks() - startTime;
    if (latency < kFrameTimeInMs) {
      SDL_Delay(kFrameTimeInMs - latency);
    }
  }

  SDL_DestroyTexture(game_pixels);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

}



