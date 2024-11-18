//
// Created by Brian Bonafilia on 10/17/24.
//


#include "mbc1.h"
#include <iostream>

uint8_t MBC1::read(uint16_t addr) {
  int bank_offset = 0x4000 * (rom_bank_index_ - 1);
  switch (addr) {
    case 0x0000 ... 0x3FFF:
      return rom_[addr];
    case 0x4000 ... 0x7FFF:
      return rom_[addr + bank_offset];
    case 0xA000 ... 0xBFFF:
      return ram_[addr - 0xA000];
  }
  return rom_[addr];
}
uint8_t MBC1::write(uint16_t addr, uint8_t val) {
  switch(addr){
    case 0x0000 ... 0x1FFF:
      if ((val & 0xF) == 0xA) {
        ram_enabled_ = true;
      }
      break;
    case 0x2000 ... 0x3FFF:
      rom_bank_index_ = val & rom_mask_;
      if (rom_bank_index_ == 0) rom_bank_index_ = 1;
      break;
    case 0xA000 ... 0xBFFF:
      ram_[addr - 0xA000] = val;
  }
  return val;
}
MBC1::MBC1(uint8_t *rom, int rom_size, int ram_size) : Mapper(rom){
  rom_size_ = rom_size;
  ram_size_ = ram_size;
  switch (rom_size_) {
    case 0:
      rom_mask_ = 0;
      break;
    case 1:
      rom_mask_ = 0x3;
      break;
    case 2:
      rom_mask_ = 0x7;
      break;
    case 3:
      rom_mask_ = 0xF;
      break;
    case 4:
      rom_mask_ = 0x1F;
      break;
    default:
      exit(1);
  }
}

