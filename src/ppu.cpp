//
// Created by Brian Bonafilia on 10/18/24.
//

#include <cstdint>
#include <cstdio>
#include "ppu.h"
#include "cpu.h"

namespace PPU {
namespace {
uint8_t *vram = new uint8_t[0x2000];
uint8_t *oam = new uint8_t[0xA0];

int current_dot = 0;
Registers registers;

void setVblankInterrupt() {
  uint8_t IF = CPU::access<CPU::read>(0xFF0F);
  IF &= 1;
  CPU::access<CPU::write>(0xFF0F, IF);
}

void drawFrame() {

  for (int i = 0; i < 18; ++i) {
    for (int j = 0; j < 20; ++j) {

    }
  }
}

void stepForwardDot() {
  ++current_dot;
  if (current_dot == 456) {
    current_dot = 0;
    ++registers.LY;
    if (registers.LY == 144) {
      setVblankInterrupt();
    } else if (registers.LY == 154) {
      registers.LY = 0;
//      drawFrame();
    }
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
  stepForwardDot();
}



}  // namespace PPU
