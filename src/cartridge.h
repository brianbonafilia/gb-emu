//
// Created by Brian Bonafilia on 10/1/24.
//

#ifndef GB_EMU_SRC_CARTRIDGE_H_
#define GB_EMU_SRC_CARTRIDGE_H_

#include <string>
#include <cstdint>
#include "cpu.h"

namespace Cartridge {

uint8_t read(uint16_t addr);
uint8_t write(uint16_t addr, uint8_t val);

void Save();

uint8_t get_bank(uint16_t addr);

void load_cartridge(const char* file_path);

}  // namespace Cartridge


#endif //GB_EMU_SRC_CARTRIDGE_H_
