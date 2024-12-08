//
// Created by Brian Bonafilia on 12/7/24.
//

#include "apu.h"
#include "cpu.h"
#include <cstdint>

namespace APU {

Registers registers{};

uint8_t access_registers(CPU::mode m, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF11:
      if (m == CPU::read) {
        return 0xFF;
      }
      registers.NR11 = val;
      return registers.NR11;
    case 0xFF16:
      if (m == CPU::read) {
        return 0xFF;
      }
      registers.NR21 = val;
      return registers.NR11;
  }
  return 0;
}

}  // namespace APU