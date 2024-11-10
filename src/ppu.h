//
// Created by Brian Bonafilia on 10/18/24.
//

#ifndef GB_EMU_SRC_PPU_H_
#define GB_EMU_SRC_PPU_H_

#include <cstdint>
#include "cpu.h"

namespace PPU {

constexpr uint32_t kWhite = 0x00FFFFFF;
constexpr uint32_t kLightGrey = 0x00a9a9a9;
constexpr uint32_t kDarkGrey = 0x00545454;
constexpr uint32_t kBlack = 0;

constexpr uint16_t kOamOffset = 0xFE00;

constexpr uint32_t kGreyPalette[4]{kWhite, kLightGrey, kDarkGrey, kBlack};

union SpriteAttributes {
  struct {
    // CGB select palette
    uint8_t cgb_palette : 3;
    // CGB select vrom bank
    uint8_t bank : 1;
    // Non-CGB only 0 = OBP0, 1 = OBP1
    uint8_t dmg_palette : 1;
    // horizontally mirror
    bool flip_x : 1;
    // vertically mirror
    bool flip_y : 1;
    // if true, BG may draw over this OBJ
    bool priority : 1;
  };
  uint8_t attr;
};

union Palette {
  struct {
    uint8_t color0: 2;
    uint8_t color1: 2;
    uint8_t color2: 2;
    uint8_t color3: 2;
  };
  uint8_t val;
};

enum PpuMode {
  hblank, vblank, oam_scan, draw
};

struct Registers {
  /* LCD Control Registers */
  union {
    struct {
      // Background & Window enable / priority
      bool bgw_ef : 1;
      // OBJ enable
      bool obj_ef : 1;
      // OBJ size
      bool obj_sz : 1;
      // BG tile map
      // 0 = 0x9800-9bFF , 1 = 0x9C00-9FFF
      bool bg_tile_map : 1;
      // BG Windows and tiles data area
      // 0 = 0x8800-97FF , 1 = 0x8000-8FFF
      bool bg_windows_tiles : 1;
      // Window enable
      bool window_enable : 1;
      // Window tile map
      // 0 = 0x9800-9bFF , 1 = 0x9C00-9FFF
      bool window_tile_map : 1;
      // LCD and PPU enable
      bool ppu_enable : 1;
    };
    uint8_t LCDC;
  };

  /* LCD Status Registers */
  // Y coordinate
  uint8_t LY;
  // LY compare
  uint8_t LYC;
  // LCD status
  union {
    struct {
      // what mode is PPU in,  (
      PpuMode mode : 2;
      bool ly_eq : 1;
      bool mode0_stat : 1;
      bool mode1_stat : 1;
      bool mode2_stat : 1;
      bool lyc_stat : 1;
    };
    uint8_t STAT;
  };
  /* Background Scrolling registers */
  uint8_t SCX;
  uint8_t SCY;

  /* Window Scroll */
  uint8_t WX;
  uint8_t WY;

  /* Palette Registers */
  // Background palette
  union {
    struct {
      uint8_t bgp_id0 : 2;
      uint8_t bgp_id1 : 2;
      uint8_t bgp_id2 : 2;
      uint8_t bgp_id3 : 2;
    };
    uint8_t BGP;
  };
  // OBJ palette 0
  union {
    struct {
      uint8_t : 2;
      uint8_t obp0_id1: 2;
      uint8_t obp0_id2: 2;
      uint8_t obp0_id3: 2;
    };
    uint8_t OBP0;
  };
  // OBJ palette 1
  union {
    struct {
      uint8_t : 2;
      uint8_t obp1_id1: 2;
      uint8_t obp1_id2: 2;
      uint8_t obp1_id3: 2;
    };
    uint8_t OBP1;
  };
};

struct PpuState {
  Registers& registers;
  uint8_t* vram;
  uint8_t* oam;
};

uint8_t read_vram(uint16_t addr);

uint8_t write_vram(uint16_t addr, uint8_t val);

uint8_t read_oam(uint16_t addr);

uint8_t write_oam(uint16_t addr, uint8_t val);

void beginDmaTransfer(uint8_t addr);

void dot();

uint8_t access_registers(CPU::mode m, uint16_t addr, uint8_t val = 0);

void set_debug(bool setting);

}  // namespace PPU

#endif //GB_EMU_SRC_PPU_H_
