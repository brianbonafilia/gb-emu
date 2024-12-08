//
// Created by Brian Bonafilia on 11/10/24.
//

#ifndef GB_EMU_SRC_RENDERING_DRAW_H_
#define GB_EMU_SRC_RENDERING_DRAW_H_

#include <cstdint>
#include "../ppu.h"

namespace PPU {

int GetTileAddr(const PpuState& state,  int tile_idx);

void DrawSprite(const PpuState& state, int tile_addr, SpriteAttributes attributes, int row, int col, uint32_t* pixels);

void DrawBackground(const PpuState& state, uint32_t* pixels);

void DrawWindow(const PpuState& state, uint32_t* pixels);

void DrawDebugScreen(const PpuState& state);

void DrawOam(const PpuState& state);

void DrawDot(const PpuState& state);

}

#endif //GB_EMU_SRC_RENDERING_DRAW_H_
