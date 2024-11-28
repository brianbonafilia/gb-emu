//
// Created by Brian Bonafilia on 11/10/24.
//

#include <cassert>
#include "draw.h"
#include "../gui.h"

namespace PPU {

namespace {

bool RowOutOfBounds(int row) {
  return row < 0 || row > 143;
}

bool ColOutOfBounds(int col) {
  return col < 0 || col > 159;
}

int GetBgOffset(const Registers& registers) {
  return registers.bg_tile_map ? 0x1C00 : 0x1800;
}

int GetWindowOffset(const Registers& registers) {
  return registers.window_tile_map ? 0x1C00 : 0x1800;
}

bool IsInWindow(const PpuState& state) {
  if (!state.registers.window_enable) {
    return false;
  }
  if (state.registers.x_pos < state.registers.WX - 8) {
    return false;
  }
  return state.registers.LY >= state.registers.WY;
}

void SetWindowTileLowHigh(const PpuState& state) {
  int window_x = state.registers.x_pos + 8 - state.registers.WX;
  int window_y = state.registers.LY - state.registers.WY;
  int x_tile = window_x / 8;
  int y_tile = window_y / 8;
  int tile_idx = state.vram[GetWindowOffset(state.registers) + x_tile + y_tile * 32];
  int tile_addr = GetTileAddr(state, tile_idx);
  int tile_row = window_y % 8;
  state.registers.bg_step = window_x % 8;
  state.registers.bg_low = state.vram[tile_addr + tile_row * 2];
  state.registers.bg_high = state.vram[tile_addr + tile_row * 2 + 1];
}

void SetBgTileLowHigh(const PpuState& state) {
  int bg_x_pos = state.registers.SCX + state.registers.x_pos;
  int bg_y_pos = state.registers.SCY + state.registers.LY;
  bg_x_pos %= 256;
  bg_y_pos %= 256;
  // Divide by 8 to get the tile location since the tiles are 8x8 pixels
  int x_tile = bg_x_pos / 8;
  int y_tile = bg_y_pos / 8;
  int tile_idx = state.vram[GetBgOffset(state.registers) + x_tile + y_tile * 32];
  int tile_addr = GetTileAddr(state, tile_idx);
  int tile_row = bg_y_pos % 8;
  state.registers.bg_step = bg_x_pos % 8;
  state.registers.bg_low = state.vram[tile_addr + tile_row * 2];
  state.registers.bg_high = state.vram[tile_addr + tile_row * 2 + 1];

}

void PushPixel(const PpuState& state) {
  int bg_step = state.registers.bg_step;
  uint8_t bg_mask = 0x80 >> bg_step;
  int color_idx = 0;
  if (bg_mask & state.registers.bg_low) color_idx++;
  if (bg_mask & state.registers.bg_high) color_idx += 2;

  int pixel = state.registers.x_pos + state.registers.LY * 160;
  switch (color_idx) {
    case 0:
      state.pixels[pixel] = kGreyPalette[state.registers.bgp_id0];
      break;
    case 1:
      state.pixels[pixel] = kGreyPalette[state.registers.bgp_id1];
      break;
    case 2:
      state.pixels[pixel] = kGreyPalette[state.registers.bgp_id2];
      break;
    case 3:
      state.pixels[pixel] = kGreyPalette[state.registers.bgp_id3];
      break;
    default:
      assert(false);

  }
//  printf("low is %X,  high is %X, bg_step is %d\n", state.registers.bg_low, state.registers.bg_high, bg_step);
//  printf("the pixel is %X, LY is %d color is %X\n", pixel, state.registers.LY, state.pixels[pixel]);

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
  Palette palette {};
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
    if (row + row_idx < 0 || row + row_idx >= 144) {
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
      } else if (col + col_idx >= 160) {
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

int MinRowVal(const Registers& registers) {
  if (registers.obj_sz) {
    return -16;
  }
  return -8;
}

void DrawOam(const PpuState& state) {
  for (int i = 0; i < 0xA0; i+=4) {
    int row = state.oam[i];
    int col = state.oam[i + 1];
    row -= 16;
    col -= 8;
    if (row < MinRowVal(state.registers) || row >= 160 ) {
      continue;
    }
    if (col < -8 || col > 168) {
      continue;
    }
    uint8_t tile_idx = state.oam[i + 2];
    SpriteAttributes attributes{.attr = state.oam[i + 3]};
    int sprite_addr = GetSpriteAddr(tile_idx);
    DrawSprite(state, sprite_addr, attributes, row, col, state.pixels);
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

void DrawDot(const PpuState& state) {
  int bg_step = state.registers.bg_step;
  int x_pos = state.registers.x_pos;
  if (x_pos >= 160) {
    state.registers.bg_step = 0;
    return;
  }
  if (!state.registers.is_in_window && IsInWindow(state)) {
    state.registers.is_in_window = true;
    SetWindowTileLowHigh(state);
  } else if (bg_step == 0) {
    if (state.registers.is_in_window) {
      SetWindowTileLowHigh(state);
    } else {
      SetBgTileLowHigh(state);
    }
  }
  PushPixel(state);
  ++state.registers.bg_step;
  state.registers.bg_step %= 8;
  ++state.registers.x_pos;
}

}  // namespace PPU