//
// Created by Brian Bonafilia on 10/18/24.
//

#include <cstdint>
#include <cstdio>
#include "ppu.h"
#include "cpu.h"
#include "gui.h"
#include "rendering/draw.h"

namespace PPU {
namespace {
uint8_t *vram = new uint8_t[0x2000];
uint8_t *oam = new uint8_t[0xA0];
uint32_t *pixels = new uint32_t[144 * 156];

int current_dot = 0;
Registers registers;
bool debug = false;

PpuState state {
    .registers = registers,
    .vram = vram,
    .oam = oam,
    .pixels = pixels,
};


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


void DrawFrame() {
  auto* pixels = new uint32_t[160 * 144];
  DrawBackground(state, pixels);
  DrawWindow(state, pixels);
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
    DrawSprite(state, sprite_addr, attributes, row, col, pixels);

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
      if (debug) DrawDebugScreen(state);
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

// Update the location of the current dot and line.
void IncrementPosition() {
  ++current_dot;
  if (current_dot == 456) {
    current_dot = 0;
    registers.x_pos = 0;
    registers.is_in_window = false;
    ++registers.LY;
    if (registers.LY == 154) {
      DrawOam(state);
      GUI::UpdateTexture(pixels);
      registers.LY = 0;
      registers.ly_eq = false;
    }
    if (registers.LY == registers.LYC) {
      if (registers.lyc_stat) {
        SetStatInterrupt();
      }
      registers.ly_eq = true;
    }
  }
}

}  // namespace

void set_debug(bool setting) {
  debug = setting;
}

uint8_t read_vram(uint16_t addr) {
  // TODO: for gameboy color eventually, needs banking
  return vram[addr - 0x8000];
}

uint8_t write_vram(uint16_t addr, uint8_t val) {
  vram[addr - 0x8000] = val;
  return vram[addr - 0x8000];
}

PpuMode GetMode() {
  if (registers.LY > 143) {
    return PpuMode::vblank;
  }
  if (current_dot < 80) {
    return PpuMode::oam_scan;
  }
  // TODO: at some point, for games which require accurate timeing we will
  // need to consider adding OBJ penalties. And making this mode variable.
  if (current_dot > 80  && current_dot < 252) {
    return PpuMode::draw;
  }
  return PpuMode::hblank;
}

void SetInterruptIfNeeded(PpuMode mode) {
  if (registers.mode0_stat && mode == hblank) {
    SetStatInterrupt();
  } else if (registers.mode1_stat && mode == vblank) {
    SetStatInterrupt();
  } else if (registers.mode2_stat && mode == oam_scan) {
    SetStatInterrupt();
  }
}

void Step() {
  IncrementPosition();
  PpuMode new_mode = GetMode();
  if (new_mode != registers.mode) {
    SetInterruptIfNeeded(new_mode);
    registers.mode = new_mode;
    if (new_mode == vblank) {
      SetVblankInterrupt();
    }
  }
  switch (registers.mode) {
    case oam_scan:
      // TODO: consider having a OAM buffer and drawing OBJs realistically
      break;
    case draw:
      DrawDot(state);
      break;
    case hblank:
    case vblank:
      // no-op
      break;
  }
}


// execute one dot
void dot() {
//  StepForwardDot();
  Step();
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
      if (m == CPU::write) {
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
        registers.WY = val;
      }
      return registers.WY;
    case 0xFF4B:
      if (m == CPU::write) {
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
