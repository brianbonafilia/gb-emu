//
// Created by Brian Bonafilia on 10/5/24.
//

#include "mapper.h"
#include <cstdio>

Mapper::Mapper(uint8_t *rom) {
  rom_ = rom;
}

uint8_t Mapper::read(uint16_t addr) {
  return rom_[addr];
}

uint8_t Mapper::write(uint16_t addr, uint8_t val) {
  printf("suprisingly attempting to write to rom, \n");
  return rom_[addr];
}
