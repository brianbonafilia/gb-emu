//
// Created by Brian Bonafilia on 10/17/24.
//


#include "mbc1.h"
#include <iostream>
#include <cassert>

uint8_t MBC1::read(uint16_t addr) {
  int bank_offset = 0x4000 * (rom_bank_index_ - 1);
  switch (addr) {
    case 0x0000 ... 0x3FFF: {
      if (advanced_banking) {
        printf("oh nozey");
        int low_offset = 0x4000 * (rom_low_bank_index);
        return rom_[(int) addr + low_offset];
      }
      return rom_[addr];
    }
    case 0x4000 ... 0x7FFF:
      return rom_[(int)addr + bank_offset];
    case 0xA000 ... 0xBFFF: {
      if (advanced_banking) {
        int low_offset = 0x4000 * (rom_low_bank_index & 0x3);
        return ram_[(int) addr - 0xA000 + low_offset];
      }
      return ram_[(int) addr - 0xA000];
    }
    default:
      assert(false);
  }
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
      if ((val & 0x1F) == 0) rom_bank_index_ = 1;
//      printf("Switching bank index to %X\n", rom_bank_index_);
      break;
    case 0x4000 ... 0x5FFF:
      printf("this would shed light?, val %X\n", val);
      rom_low_bank_index = val & rom_mask_;
    case 0x6000 ... 0x7FFF:
      printf("this would be helpful %X\n", val);
      advanced_banking = val & 1;
      break;
    case 0xA000 ... 0xBFFF:
//      printf("ok i'm writing to ram? %X : %X\n", val, addr);
      ram_[addr - 0xA000] = val;
      break;
    default:
      printf("other write detected, %X %X\n", addr, val);
  }
  return val;
}

uint8_t MBC1::get_bank(uint16_t addr) {
  switch (addr) {
    case 0x0000 ... 0x3FFF:
      if (advanced_banking) {
        return rom_low_bank_index;
      }
      return 0;
    case 0x4000 ... 0x7FFF:
      return rom_bank_index_;
    default:
      return 0;
  }
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

