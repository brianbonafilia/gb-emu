//
// Created by Brian Bonafilia on 10/5/24.
//

#ifndef GB_EMU_SRC_MAPPER_H_
#define GB_EMU_SRC_MAPPER_H_

#include <cstdint>

class Mapper {
 public:
  Mapper(uint8_t* rom);
  // Pass in the addr assume starts at 0
  virtual uint8_t read(uint16_t addr);
  virtual uint8_t write(uint16_t addr, uint8_t val);

 protected:
  uint8_t* rom_;
};


#endif //GB_EMU_SRC_MAPPER_H_
