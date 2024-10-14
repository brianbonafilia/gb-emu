//
// Created by Brian Bonafilia on 10/5/24.
//

#include "mapper.h"

Mapper::Mapper(uint8_t *rom) {
  rom_ = rom;
}

uint8_t Mapper::read(uint16_t addr) {
  return rom_[addr];
}

uint8_t Mapper::write(uint16_t addr, uint8_t val) {
  return rom_[addr] = val;
}
