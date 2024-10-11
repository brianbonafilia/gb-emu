//
// Created by Brian Bonafilia on 9/7/24.
//

#ifndef CPU_H
#define CPU_H

#include <cstdint>

namespace CPU {

/* CPU registers */
struct Registers {
  // Accumulator and flags
  union {
    struct {
      union {
        struct {
          uint8_t : 4; // nothing in first 4 flag bits
          uint8_t cf : 1; // carry flag
          uint8_t hf : 1; // half carry flag (BCD)
          uint8_t nf : 1; // subtraction flag (BCD)
          uint8_t zf : 1; // zero flag

        };
        uint8_t F;
      };
      uint8_t A;
    };
    uint16_t AF;
  };
  // 16 bit registers, which can be accessed as 8 bit registers
  union {
    struct {
      uint8_t C;
      uint8_t B;
    };
    uint16_t BC;
  };
  union {
    struct {
      uint8_t E;
      uint8_t D;
    };
    uint16_t DE;
  };
  union {
    struct {
      uint8_t L;
      uint8_t H;
    };
    uint16_t HL;
  };  // Stack Pointer
  uint16_t SP;
  // Program Counter
  uint16_t PC;

  union {
    uint8_t IE;
  };

  bool IME;
};

enum mode {
  read,
  write
};

template <mode m>
uint8_t access(uint16_t addr, uint8_t val = 0);

uint8_t** GetRegIndex();
uint16_t** GetReg16Index();
Registers& GetRegisters();

uint8_t rd8(uint16_t addr);
uint16_t rd16(uint16_t addr);
uint8_t wr8(uint16_t addr, uint8_t val);

// Power up sequence initialize registers;
void InitializeRegisters();

// Fetch the latest instructions and Execute them
void ProcessInstruction(bool debug = false);

void Tick();

}

  // namespace CPU

#endif //CPU_H
