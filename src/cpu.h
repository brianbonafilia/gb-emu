//
// Created by Brian Bonafilia on 9/7/24.
//

#ifndef CPU_H
#define CPU_H

#include <cstdint>

namespace CPU {

union Joypad {
  struct {
    bool a_or_right : 1;
    bool b_or_left : 1;
    bool select_or_up : 1;
    bool start_or_down : 1;
    bool select_dpad : 1;
    bool select_buttons : 1;
    uint8_t : 2;
  };
  struct {
    uint8_t buttons : 4;
    uint8_t : 4;
  };
  uint8_t joypad_input;

  Joypad() = default;
};

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

  uint64_t time_counter = 0;
  // Divider register,
  uint16_t DIV;
  // Timer counter
  uint8_t TIMA;
  // Timer modulo
  uint8_t TMA;
  // Timer control
  union {
    struct {
      uint8_t clock_select: 2;
      uint8_t timer_enable: 1;
      uint8_t : 5;
    };
    uint8_t TAC;
  };

  // Interrupt Enable
  union {
    struct {
      bool v_blank_ie: 1;
      bool lcd_ie: 1;
      bool time_ie: 1;
      bool serial_ie: 1;
      bool joypad_ie: 1;
      uint8_t : 3;
    };
    uint8_t IE;
  };
  // Interrupt Flags
  union {
    struct {
      bool v_blank_if: 1;
      bool lcd_if: 1;
      bool time_if: 1;
      bool serial_if: 1;
      bool joypad_if: 1;
      uint8_t : 3;
    };
    uint8_t IF;
  };

  uint8_t wram_bank = 1;

  bool halt;
  bool IME;

  Joypad controller{ .joypad_input = 0xF};
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
void InitializeRegisters(bool cgb_mode = false);

// Fetch the latest instructions and Execute them
void ProcessInstruction(bool debug = false);

void Tick();

bool Halted();

// approximation of running about a frame worth of cycles.
void RunFrame(bool debug);

}

  // namespace CPU

#endif //CPU_H
