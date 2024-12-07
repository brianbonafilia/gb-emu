//
// Created by Brian Bonafilia on 10/18/24.
//

#include <cstdint>
#include <cstdio>
#include <cassert>
#include "ppu.h"
#include "cpu.h"
#include "gui.h"
#include "rendering/draw.h"

namespace PPU {
namespace {
uint8_t *vram = new uint8_t[0x2000];
uint8_t *vram_bank1 = new uint8_t[0x2000];
uint8_t *oam = new uint8_t[0xA0];
uint32_t *pixels = new uint32_t[144 * 156];

int current_dot = 0;
Registers registers {.LCDC = 0x91};
bool debug = false;

PpuState state {
    .registers = registers,
    .vram = vram,
    .vram_bank1 = vram_bank1,
    .oam = oam,
    .pixels = pixels,
};

void SetVblankInterrupt() {
  if (!registers.ppu_enable) return;
  uint8_t IF = CPU::access<CPU::read>(0xFF0F);
  IF |= 1;
  CPU::access<CPU::write>(0xFF0F, IF);
}

void SetStatInterrupt() {
  if (!registers.ppu_enable) return;
  uint8_t IF = CPU::access<CPU::read>(0xFF0F);
  IF |= 2;
  CPU::access<CPU::write>(0xFF0F, IF);
}

void HdmaTransfer() {
  if (CPU::Halted()) {
    return;
  }
  registers.vram_dma_dest &= 0x1FFF;
  assert(registers.vram_dma_dest <= 0x1FF0);
  assert(registers.vram_dma_source < 0x7FF0 || (registers.vram_dma_source >= 0xA000 && registers.vram_dma_source <= 0xDFF0));
  registers.dma_length--;
  for (int i = 0; i < 0x10; ++i) {
    int vram_idx = registers.vram_dma_dest + i;
    int source_idx = registers.vram_dma_source + i;
    if (registers.attr_bank) {
      vram_bank1[vram_idx] = CPU::access<CPU::read>(source_idx);
    } else {
      vram[vram_idx] = CPU::access<CPU::read>(source_idx);
    }
  }
  registers.vram_dma_dest += 0x10;
  registers.vram_dma_source += 0x10;
  if (registers.dma_length == 0) {
    registers.dma_status = 0;
  }
}

void VramDmaTransfer(int length) {
  registers.vram_dma_dest &= 0x1FFF;
  printf("dest %X, source %X for length %X bank is %X dma status is %X\n",
         registers.vram_dma_dest, registers.vram_dma_source, length, registers.attr_bank, registers.dma_status);
  assert(registers.vram_dma_dest <= 0x9FF0);
  assert(registers.vram_dma_source < 0x7FF0 || (registers.vram_dma_source >= 0xA000 && registers.vram_dma_source <= 0xDFF0));
  for (int i = 0; i < length; i++) {
    assert(registers.vram_dma_dest + i < 0xA000);
    int vram_idx = registers.vram_dma_dest + i;
    int source_idx = registers.vram_dma_source + i;
    if (registers.attr_bank) {
      vram_bank1[vram_idx] = CPU::access<CPU::read>(source_idx);
    } else {
      vram[vram_idx] = CPU::access<CPU::read>(source_idx);
    }
  }
  registers.dma_status = 0;
}

void DmaTransfer(uint8_t idx) {
  for (int i = 0; i < 0xA0; ++i) {
    oam[i] = CPU::access<CPU::read>((idx << 8) | i);
  }
}

uint32_t ExtendBits(uint32_t bits) {
  return (bits << 3) | (bits >> 2);
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
      if (registers.ppu_enable) GUI::UpdateTexture(pixels);
      DrawDebugScreen(state);
      registers.LY = 0;
      registers.ly_eq = false;
    }
    if (registers.LY == registers.LYC) {
      if (registers.lyc_stat) {
        SetStatInterrupt();
      }
      registers.ly_eq = true;
    } else {
      registers.ly_eq = false;
    }
  }
}

}  // namespace

void set_debug(bool setting) {
  debug = setting;
}

uint8_t read_vram(uint16_t addr) {
  if (registers.attr_bank) {
    return vram_bank1[addr - 0x8000];
  }
  return vram[addr - 0x8000];
}

uint8_t write_vram(uint16_t addr, uint8_t val) {
  if (registers.attr_bank) {
    vram_bank1[addr - 0x8000] = val;
    return vram_bank1[addr - 0x8000];
  }
  vram[addr - 0x8000] = val;
  return vram[addr - 0x8000];
}

PpuMode GetMode() {
  if (registers.LY > 143) {
    return PpuMode::vblank;
  }
  if (current_dot <= 80) {
    return PpuMode::oam_scan;
  }
  // TODO: at some point, for games which require accurate timeing we will
  // need to consider adding OBJ penalties. And making this mode variable.
  if (current_dot < 252) {
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
  if (!registers.ppu_enable) {
    return;
  }
  IncrementPosition();
  PpuMode new_mode = GetMode();
  if (new_mode != registers.mode) {
    SetInterruptIfNeeded(new_mode);
    registers.mode = new_mode;
    if (new_mode == vblank) {
      SetVblankInterrupt();
    } else if (new_mode == hblank && registers.hdma_transfer) {
      // transfer 0x10 bytes as part of transfer.
      HdmaTransfer();
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
  Step();
}

uint8_t access_registers(CPU::mode m, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF40:
      if (m == CPU::write) {
        bool old_ppu = registers.ppu_enable;
        registers.LCDC = val;
        if (!registers.ppu_enable) {
          printf("turning off ppu %X\n", registers.LCDC);
          registers.LY = 0;
          registers.current_dot = 0;
          registers.mode = hblank;
        } else if (!old_ppu) {
          printf("turning on ppu %X\n", registers.LCDC);
        }
      }
      return registers.LCDC;
    case 0xFF41:
      if (m == CPU::write) {
        printf("writing to STAT %X\n", val);
        val &= 0xFC;
        registers.STAT |= val;
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
    case 0xFF4D:
      printf("accessing double speed with val %X mode %d\n", val, m);
      return 0;
      //assert((val & 1) == 0);
//      assert(false);
    case 0xFF4F:
      if (m == CPU::write) {
        registers.attr_bank = val & 1;
      }
      return registers.attr_bank;
    case 0xFF51:
      if (m == CPU::write) {
        registers.vram_dma_source &= 0x00FF;
        registers.vram_dma_source |= val << 8;
      }
      printf("writing to $FF51 val %X\n", val);
      return 0;
    case 0xFF52:
      if (m == CPU::write) {
        val &= 0xF0;
        registers.vram_dma_source &= 0xFF00;
        registers.vram_dma_source |= val;
      }
      printf("writing to $FF52 val %X\n", val);
      return 0;
    case 0xFF53:
      if (m == CPU::write) {
        registers.vram_dma_dest &= 0x00FF;
        registers.vram_dma_dest |= val << 8;
      }
      printf("writing to $FF53 val %X\n", val);
      return 0;
    case 0xFF54:
      if (m == CPU::write) {
        val &= 0xF0;
        registers.vram_dma_dest &= 0xFF00;
        registers.vram_dma_dest |= val;
      }
      printf("writing to $FF54 val %X\n", val);
      return 0;
    case 0xFF55:
      if (m == CPU::read) {
        if (registers.dma_status == 0) {
          return 0xFF;
        }
      } else {
        if (registers.hdma_started && (val & 0x80) == 0) {
          registers.hdma_started = false;
          return 0;
        }
        registers.dma_status = val;
        int length = ((val & 0x74) + 1) * 0x10;
        if (!registers.hdma_transfer) {
          VramDmaTransfer(length);
        } else {
          registers.hdma_started = true;
        }
      }
      return registers.dma_status;
    case 0xFF68:
      if (m == CPU::write) {
        registers.bcps = val;
      }
      return registers.bcps;
    case 0xFF69:
      if (m == CPU::write) {
        registers.bg_cram[registers.bg_color_addr] = val;
        if (registers.bg_auto_increment_color_addr) {
          registers.bg_color_addr++;
        }
        return val;
      }
      return registers.bg_cram[registers.bg_color_addr];
    case 0xFF6A:
      if (m == CPU::write) {
        registers.ocps = val;
      }
      return registers.ocps;
    case 0xFF6B:
      if (m == CPU::write) {
        registers.obj_cram[registers.obj_color_addr] = val;
        if (registers.obj_auto_increment_color_addr) {
          registers.obj_color_addr++;
        }
        return val;
      }
      return registers.bg_cram[registers.obj_color_addr];
    default:
      return 0x00;
  }
}

uint8_t write_oam(uint16_t addr, uint8_t val) {
  oam[addr - kOamOffset] = val;
  return val;
}

uint8_t read_oam(uint16_t addr) {
  return oam[addr - kOamOffset];
}

void set_cgb_mode(bool cgb_mode) {
  registers.cgb_mode = cgb_mode;
}


uint32_t ToRgb888(Color c) {
  return (ExtendBits(c.red()) << 16) | (ExtendBits(c.green()) << 8) | ExtendBits(c.blue());
}

}  // namespace PPU
