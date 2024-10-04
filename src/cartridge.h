//
// Created by Brian Bonafilia on 10/1/24.
//

#ifndef GB_EMU_SRC_CARTRIDGE_H_
#define GB_EMU_SRC_CARTRIDGE_H_

#include <string>
#include <cstdint>

namespace Cartridge {

uint8_t access(uint16_t addr, uint8_t val = 0);

void load_cartridge(const char* file_path);

}  // namespace Cartridge


#endif //GB_EMU_SRC_CARTRIDGE_H_
