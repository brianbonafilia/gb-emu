//
// Created by Brian Bonafilia on 10/5/24.
//

#include "mapper.h"
#include <cstdio>

Mapper::Mapper(uint8_t *rom) {
  rom_ = rom;
  ram_ = new uint8_t[0x2000];
}

Mapper::~Mapper() {
  delete [] ram_;
}

uint8_t Mapper::read(uint16_t addr) {
  if (addr >= 0xA000) {
    return ram_[addr - 0xA000];
  }
  return rom_[addr];
}

uint8_t Mapper::write(uint16_t addr, uint8_t val) {
  if (addr >= 0xA000) {
    ram_[addr - 0xA000] = val;
    return val;
  }
  return rom_[addr];
}


