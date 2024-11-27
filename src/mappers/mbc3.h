//
// Created by Brian Bonafilia on 11/24/24.
//

#ifndef GB_EMU_SRC_MAPPERS_MBC3_H_
#define GB_EMU_SRC_MAPPERS_MBC3_H_

#include "../mapper.h"
class MBC3 : public Mapper {
 public:
  explicit MBC3(uint8_t* rom, uint8_t* ram, int rom_size, int ram_size);

  uint8_t read(uint16_t addr) override;
  uint8_t write(uint16_t addr, uint8_t val) override;
  uint8_t get_bank(uint16_t addr) override;

 private:
  bool advanced_banking = false;
  bool ram_enabled_ = false;
  int rom_bank_index_ = 1;
  int rom_low_bank_index = 0;
  int rom_size_;
  int ram_size_;
  uint8_t rom_mask_;

};

#endif //GB_EMU_SRC_MAPPERS_MBC3_H_
