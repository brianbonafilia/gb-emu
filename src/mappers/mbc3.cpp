//
// Created by Brian Bonafilia on 11/24/24.
//

//
// Created by Brian Bonafilia on 10/17/24.
//


#include "mbc3.h"
#include <iostream>
#include <cassert>

uint8_t MBC3::read(uint16_t addr) {
  int bank_offset = 0x4000 * (rom_bank_index_ - 1);
  switch (addr) {
    case 0x0000 ... 0x3FFF:
      return rom_[addr];
    case 0x4000 ... 0x7FFF:
      return rom_[addr + bank_offset];
    case 0xA000 ... 0xBFFF: {
      if (!ram_enabled_) {
        return 0xFF;
      }
      int low_offset = 0x2000 * (rom_low_bank_index & 0x3);
      return ram_[addr - 0xA000 + low_offset];
    }
    default:
      assert(false);
  }
}
uint8_t MBC3::write(uint16_t addr, uint8_t val) {
  switch(addr){
    case 0x0000 ... 0x1FFF:
      if ((val & 0xF) == 0xA) {
        ram_enabled_ = true;
      }
      break;
    case 0x2000 ... 0x3FFF:
      rom_bank_index_ = val & rom_mask_;
      if (val == 0) {
        rom_bank_index_ = 1;
      }
      break;
    case 0x4000 ... 0x5FFF:
      rom_low_bank_index = val;
    case 0x6000 ... 0x7FFF:
      advanced_banking = val & 1;
      break;
    case 0xA000 ... 0xBFFF: {
      if (!ram_enabled_) {
        return 0xFF;
      }
      int low_offset = 0x2000 * (rom_low_bank_index & 0x3);
      ram_[addr - 0xA000 + low_offset] = val;
      return val;
    }
    default:
      printf("other write detected, %X %X\n", addr, val);
  }
  return val;
}

uint8_t MBC3::get_bank(uint16_t addr) {
  switch (addr) {
    case 0x0000 ... 0x3FFF:
      return 0;
    case 0x4000 ... 0x7FFF:
      return rom_bank_index_;
    case 0xA000 ... 0xBFFF:
      return rom_low_bank_index;
    default:
      return 0;
  }
}

MBC3::MBC3(uint8_t *rom, uint8_t* ram, int rom_size, int ram_size) : Mapper(rom, ram){
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
    case 5:
      rom_mask_ = 0x3F;
      break;
    case 6:
      rom_mask_ = 0x7F;
      break;
    default:
      exit(1);
  }
}