//
// Created by Brian Bonafilia on 10/17/24.
//

#ifndef GB_EMU_SRC_MAPPERS_MBC1_H_
#define GB_EMU_SRC_MAPPERS_MBC1_H_

#include "../mapper.h"
class MBC1 : public Mapper {
 public:
  explicit MBC1(uint8_t* rom) : Mapper(rom) {};

  uint8_t read(uint16_t addr) override;
  uint8_t write(uint16_t addr, uint8_t val) override;

 private:
  bool ram_enabled_ = false;
  int rom_bank_index_ = 1;

};


#endif //GB_EMU_SRC_MAPPERS_MBC1_H_
