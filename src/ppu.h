//
// Created by Brian Bonafilia on 10/18/24.
//

#ifndef GB_EMU_SRC_PPU_H_
#define GB_EMU_SRC_PPU_H_

#include <cstdint>

namespace PPU {

struct Registers {
  /* LCD Control Registers */
  union {
    struct {
      // Background & Window enable / priority
      bool bgw_ef;
      // OBJ enable
      bool obj_ef;
      // OBJ size
      bool obj_sz;
      // BG tile map
      bool bg_tile_map;
      // BG Windows and tiles
      bool bg_windows_tiles;
      // Window enable
      bool window_enable;
      // Window tile map
      bool window_tile_map;
      // LCD and PPU enable
      bool ppu_enable;
    };
    uint8_t LCDC;
  };

  /* LCD Status Registers */
  // Y coordinate
  uint8_t LY;
  // LY compare
  uint8_t LYC;
  // LCD status
  uint8_t STAT;

  /* Background Scrolling registers */
  uint8_t SCX;
  uint8_t SCY;

  /* Window Scroll */
  uint8_t WX;
  uint8_t WY;

  /* Palette Registers */
  // Background palette
  uint8_t BGP;
  // OBJ palette 0
  uint8_t OBP0;
  // OBJ palette 1
  uint8_t OBP1;
};

uint8_t read_vram(uint16_t addr);

uint8_t write_vram(uint16_t addr, uint8_t val);

uint8_t read_oam();

void beginDmaTransfer(uint8_t addr);

void dot();

}  // namespace PPU

#endif //GB_EMU_SRC_PPU_H_
