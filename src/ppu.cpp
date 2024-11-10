//
// Created by Brian Bonafilia on 10/18/24.
//

#include <cstdint>
#include <cstdio>
#include <cassert>
#include "ppu.h"
#include "cpu.h"
#include "gui.h"

namespace PPU {
namespace {
uint8_t *vram = new uint8_t[0x2000];
uint8_t *oam = new uint8_t[0xA0];

constexpr uint32_t kWhite = 0x00FFFFFF;
constexpr uint32_t kLightGrey = 0x00a9a9a9;
constexpr uint32_t kDarkGrey = 0x00545454;
constexpr uint32_t kBlack = 0;

constexpr uint16_t kOamOffset = 0xFE00;

constexpr uint32_t kGreyPalette[4]{kWhite, kLightGrey, kDarkGrey, kBlack};


int current_dot = 0;
Registers registers;

void SetVblankInterrupt() {
  uint8_t IF = CPU::access<CPU::read>(0xFF0F);
  IF |= 1;
  CPU::access<CPU::write>(0xFF0F, IF);
}

void SetStatInterrupt() {
  uint8_t IF = CPU::access<CPU::read>(0xFF0F);
  IF |= 2;
  CPU::access<CPU::write>(0xFF0F, IF);
}

bool RowOutOfBounds(int row) {
  return row < 0 || row > 143;
}

bool ColOutOfBounds(int col) {
  return col < 0 || col > 159;
}

void DrawTile( int tileAddr, int x, int y, uint32_t* pixels) {
  for (int row = 0; row < 8; ++row) {
    if (RowOutOfBounds(row + y)) continue;
    uint8_t left_byte = vram[tileAddr + row * 2];
    uint8_t right_byte = vram[tileAddr + row * 2 + 1];
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
          pixels[pixel] = kGreyPalette[registers.bgp_id0];
          break;
        case 1:
          pixels[pixel] = kGreyPalette[registers.bgp_id1];
          break;
        case 2:
          pixels[pixel] = kGreyPalette[registers.bgp_id2];
          break;
        case 3:
          pixels[pixel] = kGreyPalette[registers.bgp_id3];
          break;
        default:
          assert(false);
      }
      --shift;
      mask >>= 1;
    }
  }
}

int GetTileAddr(int tile_idx) {
  if (registers.bg_windows_tiles) {
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

void DrawSprite(int tile_addr, SpriteAttributes attributes, int row, int col, uint32_t* pixels) {
  Palette palette;
  if (attributes.dmg_palette) {
    palette.val = registers.OBP1;
  } else {
    palette.val = registers.OBP0;
  }
  int sprite_rows = 8;
  int sprite_cols = 8;
  if (registers.obj_sz) {
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
    uint16_t left_byte = vram[tile_addr + sprite_row * 2];
    uint16_t right_byte = vram[tile_addr + sprite_row * 2 + 1];
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

void DrawBackground(uint32_t* pixels) {
  int tile_map_offset = registers.bg_tile_map ? 0x1C00 : 0x1800;
  for (int row = 0; row < 18; ++row) {
    for (int col = 0; col < 20; ++col) {
      uint8_t tile_idx = vram[tile_map_offset + (row * 32) + col];
      int vram_location = GetTileAddr(tile_idx);
      DrawTile(vram_location, col * 8, row * 8, pixels);
    }
  }
}

void DrawWindow(uint32_t* pixels) {
  if (!registers.window_enable) {
    return;
  }
  int tile_map_offset = registers.window_tile_map ? 0x1C00 : 0x1800;
  int colOffset = registers.WX - 7;
  int rowOffset = registers.WY;
  for (int row = 0; row < 18; ++row) {
    for (int col = 0; col < 20; ++col) {
      uint8_t tile_idx = vram[tile_map_offset + (row * 32) + col];
      int vram_location = GetTileAddr(tile_idx);
      DrawTile(vram_location, col * 8 + colOffset, row * 8 + rowOffset, pixels);
    }
  }
}

void DrawDebugTile( int tileAddr, int x, int y, uint32_t* pixels) {
  for (int row = 0; row < 8; ++row) {
    uint8_t left_byte = vram[tileAddr + row * 2];
    uint8_t right_byte = vram[tileAddr + row * 2 + 1];
    uint8_t mask = 0x80;
    int shift = 7;
    for (int col = 0; col < 8; ++col) {
      int pixel = ((y + row) * 256) + x + col;
      int val = 0;
      if (left_byte & mask) val += 1;
      if (right_byte & mask) val += 2;
      switch (val) {
        case 0:
          pixels[pixel] = kGreyPalette[registers.bgp_id0];
          break;
        case 1:
          pixels[pixel] = kGreyPalette[registers.bgp_id1];
          break;
        case 2:
          pixels[pixel] = kGreyPalette[registers.bgp_id2];
          break;
        case 3:
          pixels[pixel] = kGreyPalette[registers.bgp_id3];
          break;
        default:
          assert(false);
      }
      --shift;
      mask >>= 1;
    }
  }
}

void DrawDebugScreen() {
  auto* pixels = new uint32_t[256 * 512];
  for(int row = 0; row < 64; ++row) {
    for (int col = 0; col < 32; ++col) {
      uint8_t tile_idx = vram[0x1800 + (row * 32) + col];
      int vram_location = GetTileAddr(tile_idx);
      DrawDebugTile(vram_location, col * 8, row * 8, pixels);
    }
  }
  GUI::DrawDebugScreen(pixels);
}

void DrawFrame() {
  auto* pixels = new uint32_t[160 * 144];
  DrawBackground(pixels);
  DrawWindow(pixels);
  for (int i = 0; i < 0xA0; i+=4) {
    int row = oam[i];
    int col = oam[i + 1];
    row -= 16;
    col -= 8;
    if (row < 8 || row >= 160 ) {
      continue;
    }
    if (col < -8 || col > 168) {
      continue;
    }
    uint8_t tile_idx = oam[i + 2];
    SpriteAttributes attributes{.attr = oam[i + 3]};
    int sprite_addr = GetSpriteAddr(tile_idx);
    DrawSprite(sprite_addr, attributes, row, col, pixels);

  }
  GUI::UpdateTexture(pixels);
  delete []pixels;
}

void HandleStat() {
  if (current_dot == 0) {
    if (registers.mode1_stat) {
      SetStatInterrupt();
    }
    if (registers.LY < 143) {
      registers.mode = PpuMode::oam_scan;
    }
  } else if (current_dot == 80) {
    if (registers.LY < 143) {
      registers.mode = PpuMode::draw;
    }
  } else if (current_dot == 172) {
    SetStatInterrupt();
    if (registers.LY < 143) {
      registers.mode = PpuMode::hblank;
    }
  }
}

void StepForwardDot() {
  ++current_dot;

  if (current_dot == 456) {
    current_dot = 0;
    ++registers.LY;
    if (registers.LY == 60) {
      DrawFrame();
      DrawDebugScreen();
    }
    if (registers.LY == 144) {
      registers.mode = PpuMode::vblank;
      SetVblankInterrupt();
      if (registers.mode1_stat) {
        SetStatInterrupt();
      }
    } else if (registers.LY == 154) {
      registers.LY = 0;
    }
    if (registers.lyc_stat && registers.LY == registers.LYC) {
      SetStatInterrupt();
    }
  }
  HandleStat();
}

void DmaTransfer(uint8_t idx) {
  for (int i = 0; i < 0xA0; ++i) {
    oam[i] = CPU::access<CPU::read>((idx << 8) | i);
  }
}

}  // namespace


uint8_t read_vram(uint16_t addr) {
  // TODO: for gameboy color eventually, needs banking
  return vram[addr - 0x8000];
}

uint8_t write_vram(uint16_t addr, uint8_t val) {
  vram[addr - 0x8000] = val;
  return vram[addr - 0x8000];
}

// execute one dot
void dot() {
  StepForwardDot();
}

uint8_t access_registers(CPU::mode m, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF10 ... 0xFF3F:
      return 0x90;
    case 0xFF40:
      if (m == CPU::write) {
        registers.LCDC = val;
      }
      return registers.LCDC;
    case 0xFF41:
      if (m == CPU::write) {
        registers.STAT = val;
      }
      return registers.STAT;
    case 0xFF42:
      if (m == CPU::write) {
        registers.SCY = val;
      }
      return registers.SCY;
    case 0xFF43:
      if (m == CPU::write) {
        registers.SCX = val;
      }
      return registers.SCX;
    case 0xFF44:
      return registers.LY;
    case 0xFF45:
      if (m == registers.LYC) {
        registers.LYC = val;
      }
      return registers.LYC;
    case 0xFF46:
      if (m == CPU::write) {
        DmaTransfer(val);
      }
      return 0xFF;
    case 0xFF47:
      if (m == CPU::write) {
        registers.BGP = val;
      }
      return registers.BGP;
    case 0xFF48:
      if (m == CPU::write) {
        registers.OBP0 = val;
      }
      return registers.OBP0;
    case 0xFF49:
      if (m == CPU::write) {
        registers.OBP1 = val;
      }
      return registers.OBP1;
    case 0xFF4A:
      if (m == CPU::write) {
        printf("some trickery up in dis %X \n", val);
        registers.WY = val;
      }
      return registers.WY;
    case 0xFF4B:
      if (m == CPU::write) {
        printf("some trickery up in dis %X \n", val);
        registers.WX = val;
      }
      return registers.WX;
    default:
      return 0x90;
  }
}

uint8_t write_oam(uint16_t addr, uint8_t val) {
  oam[addr - kOamOffset] = val;
  return val;
}

uint8_t read_oam(uint16_t addr) {
  return oam[addr - kOamOffset];
}

}  // namespace PPU
