//
// Created by Brian Bonafilia on 10/6/24.
//

#include <SDL2/SDL.h>
#include <iostream>
#include <cstdint>
#include <random>
#include <cassert>
#include "cpu.h"

namespace GUI {

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *game_pixels;

constexpr int kScreenWidth = 800;
constexpr int kSreenHeight = 720;

constexpr int kPixelWidth = 160;
constexpr int kPixelHeight = 144;

CPU::Joypad actions{.joypad_input = 0xFF};
CPU::Joypad direction{.joypad_input = 0xFF};

// May make this more accurate i guess later, probably does not matter. Currently 60FPS.
constexpr uint32_t kFrameTimeInMs = 16;

void UpdateTexture(uint32_t* pixels) {
  SDL_UpdateTexture(game_pixels, nullptr, pixels,kPixelWidth * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, game_pixels, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void SetControllerState(CPU::Joypad& controller) {
  controller.buttons = 0xF;
  if ((controller.joypad_input & 0xF0) == 0x30) {
    controller.joypad_input = 0x3F;
  } else if ((controller.joypad_input & 0xF0) == 0x20) {
    controller.buttons = direction.buttons;
  } else if ((controller.joypad_input & 0xF0) == 0x10) {
    controller.buttons = actions.buttons;
  }
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
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, kPixelWidth, kPixelHeight);


  bool is_running = true;

  while (is_running) {
    SDL_Event e;
    uint32_t startTime = SDL_GetTicks();
    actions.buttons = 0xF;
    direction.buttons = 0xF;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        is_running = false;
      } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
          case SDLK_UP:
            direction.select_or_up = false;
            break;
          case SDLK_DOWN:
            direction.start_or_down = false;
            break;
          case SDLK_LEFT:
            direction.b_or_left = false;
            break;
          case SDLK_RIGHT:
            direction.a_or_right = false;
            break;
          case SDLK_SPACE:
            actions.a_or_right = false;
            break;
          case SDLK_x:
            actions.b_or_left = false;
            break;
          case SDLK_RETURN:
            actions.start_or_down = false;
            break;
          case SDLK_c:
            actions.select_or_up = false;
            break;
        }
      }
    }
    CPU::RunFrame(debug);
    uint32_t latency = SDL_GetTicks() - startTime;
    if (latency < kFrameTimeInMs) {
      SDL_Delay(kFrameTimeInMs - latency);
    } else {
      printf("poopy pants\n");
    }
  }

  SDL_DestroyTexture(game_pixels);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

}



