//
// Created by Brian Bonafilia on 12/6/24.
//

#ifndef GB_EMU_SRC_APU_H_
#define GB_EMU_SRC_APU_H_

#include <stdint.h>
#include "cpu.h"

namespace APU {

struct Registers {
 uint8_t NR11;
 uint8_t NR21;
};

uint8_t access_registers(CPU::mode m, uint16_t addr, uint8_t val);

}

#endif //GB_EMU_SRC_APU_H_
