//
// Created by Brian Bonafilia on 11/10/24.
//

#include <cassert>
#include "draw.h"
#include "../ppu.h"
#include "../gui.h"

namespace PPU {

namespace {

bool RowOutOfBounds(int row) {
  return row < 0 || row > 143;
}

bool ColOutOfBounds(int col) {
  return col < 0 || col > 159;
}

}  // namespace

void DrawTile(const PpuState& state, int tileAddr, int x, int y, uint32_t *pixels) {
  for (int row = 0; row < 8; ++row) {
    if (RowOutOfBounds(row + y)) continue;
    uint8_t left_byte = state.vram[tileAddr + row * 2];
    uint8_t right_byte = state.vram[tileAddr + row * 2 + 1];
    uint8_t mask = 0x80;
    int shift = 7;
    for (int col = 0; col < 8; ++col) {
      if (ColOutOfBounds(col + x)) continue;
      int pixel = ((y + row) * 160) + x + col;
      int val = 0; // ((left_byte & mask)| ((right_byte & mask) >> (shift-1));
      if (left_byte & mask) val += 1;
      if (right_byte & mask) val += 2;
      switch (val) {
        case 0:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id0];
          break;
        case 1:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id1];
          break;
        case 2:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id2];
          break;
        case 3:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id3];
          break;
        default:
          assert(false);
      }
      --shift;
      mask >>= 1;
    }
  }
}

int GetTileAddr(const PpuState& state, int tile_idx) {
  if (state.registers.bg_windows_tiles) {
    return 16 * tile_idx;
  } else {
    // treat base addr as 0x1000 and use index as signed 8 bit index into tile data (16 byte tiles)
    auto val = (int8_t) tile_idx;
    return 0x1000 + val * 16;
  }
}

int GetSpriteAddr(int tile_idx) {
  return tile_idx * 16;
}

void DrawSprite(const PpuState& state, int tile_addr, SpriteAttributes attributes, int row, int col, uint32_t *pixels) {
  Palette palette;
  if (attributes.dmg_palette) {
    palette.val = state.registers.OBP1;
  } else {
    palette.val = state.registers.OBP0;
  }
  int sprite_rows = 8;
  int sprite_cols = 8;
  if (state.registers.obj_sz) {
    sprite_rows = 16;
  }
  for (int row_idx = 0; row_idx < sprite_rows; ++row_idx) {
    if (row + row_idx < 0) {
      continue;
    }
    int sprite_row = row_idx;
    if (attributes.flip_y) {
      sprite_row = sprite_rows - row_idx;
    }
    uint16_t left_byte = state.vram[tile_addr + sprite_row * 2];
    uint16_t right_byte = state.vram[tile_addr + sprite_row * 2 + 1];
    uint16_t mask = 0x80;
    if (attributes.flip_x) {
      mask = 0x1;
    }
    int shift = 7;
    for (int col_idx = 0; col_idx < sprite_cols; ++col_idx) {
      if (col + col_idx < 0) {
        continue;
      }
      int pixel = ((row + row_idx) * 160) + col + col_idx;
      int val = 0;
      if (left_byte & mask) val += 1;
      if (right_byte & mask) val += 2;
      switch (val) {
        case 0:
          break;
        case 1:
          pixels[pixel] = kGreyPalette[palette.color1];
          break;
        case 2:
          pixels[pixel] = kGreyPalette[palette.color2];
          break;
        case 3:
          pixels[pixel] = kGreyPalette[palette.color3];
          break;
        default:
          assert(false);
      }
      --shift;
      if (attributes.flip_x) {
        mask <<= 1;
      } else {
        mask >>= 1;
      }
    }
  }
}

void DrawBackground(const PpuState& state, uint32_t *pixels) {
  int tile_map_offset = state.registers.bg_tile_map ? 0x1C00 : 0x1800;
  for (int row = 0; row < 18; ++row) {
    for (int col = 0; col < 20; ++col) {
      uint8_t tile_idx = state.vram[tile_map_offset + (row * 32) + col];
      int vram_location = GetTileAddr(state, tile_idx);
      DrawTile(state, vram_location, col * 8, row * 8, pixels);
    }
  }
}

void DrawWindow(const PpuState& state, uint32_t *pixels) {
  if (!state.registers.window_enable) {
    return;
  }
  int tile_map_offset = state.registers.window_tile_map ? 0x1C00 : 0x1800;
  int colOffset = state.registers.WX - 7;
  int rowOffset = state.registers.WY;
  for (int row = 0; row < 18; ++row) {
    for (int col = 0; col < 20; ++col) {
      uint8_t tile_idx = state.vram[tile_map_offset + (row * 32) + col];
      int vram_location = GetTileAddr(state, tile_idx);
      DrawTile(state, vram_location, col * 8 + colOffset, row * 8 + rowOffset, pixels);
    }
  }
}

void DrawDebugTile(const PpuState& state, int tileAddr, int x, int y, uint32_t *pixels) {
  for (int row = 0; row < 8; ++row) {
    uint8_t left_byte = state.vram[tileAddr + row * 2];
    uint8_t right_byte = state.vram[tileAddr + row * 2 + 1];
    uint8_t mask = 0x80;
    int shift = 7;
    for (int col = 0; col < 8; ++col) {
      int pixel = ((y + row) * 256) + x + col;
      int val = 0;
      if (left_byte & mask) val += 1;
      if (right_byte & mask) val += 2;
      switch (val) {
        case 0:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id0];
          break;
        case 1:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id1];
          break;
        case 2:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id2];
          break;
        case 3:
          pixels[pixel] = kGreyPalette[state.registers.bgp_id3];
          break;
        default:
          assert(false);
      }
      --shift;
      mask >>= 1;
    }
  }
}

void DrawDebugScreen(const PpuState& state) {
  auto *pixels = new uint32_t[256 * 512];
  for (int row = 0; row < 64; ++row) {
    for (int col = 0; col < 32; ++col) {
      uint8_t tile_idx = state.vram[0x1800 + (row * 32) + col];
      int vram_location = GetTileAddr(state, tile_idx);
      DrawDebugTile(state, vram_location, col * 8, row * 8, pixels);
    }
  }
  GUI::DrawDebugScreen(pixels);
}

}  // namespace PPU