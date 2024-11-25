//
// Created by Brian Bonafilia on 10/5/24.
//

#ifndef GB_EMU_SRC_MAPPER_H_
#define GB_EMU_SRC_MAPPER_H_

#include <cstdint>

class Mapper {
 public:
  explicit Mapper(uint8_t* rom);
  ~Mapper();
  // Pass in the addr assume starts at 0
  virtual uint8_t read(uint16_t addr);
  virtual uint8_t write(uint16_t addr, uint8_t val);
  virtual uint8_t get_bank(uint16_t addr);

 protected:
  uint8_t* rom_;
  uint8_t* ram_;
};


#endif //GB_EMU_SRC_MAPPER_H_
