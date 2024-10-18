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
      rom_bank_index_ = val & 3;
      if (rom_bank_index_ == 0) rom_bank_index_ = 1;
      break;
  }
  return val;
}

