//
// Created by Brian Bonafilia on 9/7/24.
//
#include <cstdint>
#include <iostream>
#include <functional>
#include <cassert>
#include "cpu.h"
#include "alu.h"
#include "ppu.h"
#include "cartridge.h"
#include "gui.h"
#include "debug/log.h"

namespace CPU {
namespace {

bool debug = false;
int tick_count = 0;

Registers registers;
uint8_t **reg_ind = new uint8_t *[7];
uint16_t **reg_16ind = new uint16_t *[4];
// start with 8KiB, will have to update to support GBC banking later
uint8_t *wram = new uint8_t[0x2000];
uint8_t *hram = new uint8_t[0x7E];
uint8_t *serial_port = new uint8_t[0x2];

bool next_op_ready = false;

// amount of cycles in a frame, 114 per scanline, with 154 scanlines
constexpr int kTotalCycles = 17556;
int remaining_cycles = 0;
int serial_interrupt_counter = 0;

bool found_break = false;
uint16_t next_break = 0xC6A0;
}  //  namespace

template<mode m>
uint8_t access(uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0x0000 ... 0X3FFF:
      if (m == read) {
        return Cartridge::read(addr);
      }
      return Cartridge::write(addr, val);
    case 0x4000 ... 0x7FFF:
      if (m == read) {
        return Cartridge::read(addr);
      }
      return Cartridge::write(addr, val);
    case 0x8000 ... 0x9FFF:
      if (m == write) {
        return PPU::write_vram(addr, val);
      }
      return PPU::read_vram(addr);
    case 0XA000 ... 0xBFFF:
      if (m == write) {
        return Cartridge::write(addr, val);
      }
      return Cartridge::read(addr);
    case 0xC000 ... 0xCFFF:
      // Work RAM
      if (m == write) {
        wram[addr - 0xC000] = val;
      }
      return wram[addr - 0xC000];
    case 0xD000 ... 0xDFFF:
      // WRAM which is banked for GBC
      if (m == write) {
        wram[addr - 0xC000] = val;
      }
      return wram[addr - 0xC000];
    case 0xE000 ... 0xFDFF:
      printf("testing?\n");
      // Not supposed to go here
      return 0;
    case 0xFE00 ... 0xFE9F:
      if (m == write) {
        return PPU::write_oam(addr, val);
      }
      return PPU::read_oam(addr);
    case 0xFEA0 ... 0xFEFF:
      // Not usable
      return 0;
    case 0XFF00:
      if (m == write) {
        val &= 0xF0;
        registers.controller.joypad_input &= 0xF;
        registers.controller.joypad_input |= val;
      }
      return registers.controller.joypad_input;
    case 0xFF01:
      if (m == write) {
        serial_port[0] = val;
      }
      return 0xFF;
    case 0xFF02:
      if (m == write) {
        serial_port[1] = val;
        if (val == 0x81) {
          std::cout << serial_port[0];
          serial_interrupt_counter = 8;
        }
      }
    case 0xFF03:
      // unknown at the moment
      return 0;
    case 0xFF04:
      if (m == write) {
        registers.DIV = 0;
      }
      return registers.DIV;
    case 0xFF05:
      if (m == write) {
        registers.TIMA = val;
      }
      return registers.TIMA;
    case 0xFF06:
      if (m == write) {
        registers.TMA = val;
      }
      return registers.TMA;
    case 0xFF07:
      if (m == write) {
        registers.TAC = val;
      }
      return registers.TAC;
    case 0xFF08 ... 0xFF0E:
      return 0x90;
    case 0xFF0F:
      if (m == write) {
        registers.IF = val;
      }
      return registers.IF;
    case 0xFF10 ... 0xFF7F:
      return PPU::access_registers(m, addr, val);
    case 0xFF80 ... 0xFFFE:
      // High RAM
      if (m == write) {
        hram[addr - 0xFF80] = val;
      }
      return hram[addr - 0xFF80];
    case 0xFFFF:
      if (m == write) {
        registers.IE = val;
      }
      return registers.IE;
  }
  printf("hmmm???\n");
  return 0;
}

uint8_t imm8() {
  uint8_t val = rd8(registers.PC++);
  return val;
}

uint16_t imm16() {
  uint16_t val = rd16(registers.PC);
  registers.PC += 2;
  return val;
}

uint16_t rd16(uint16_t addr) {
  Tick();
  Tick();
  return access<read>(addr) | (access<read>(addr + 1) << 8);
}

uint8_t rd8(uint16_t addr) {
  Tick();
  return access<read>(addr);
}

uint8_t wr8(uint16_t addr, uint8_t val) {
//  printf("Writing  0x%X to 0x%X \n", val, addr);
  Tick();
  return access<write>(addr, val);
}

// Check if the last increment caused a timer to overflow
void CheckTimer() {
  if (registers.TIMA == 0x0) {
    registers.TIMA = registers.TMA;
    registers.time_if = true;
  }
}

// TODO: evaluate running 4 M-cycles instead of one 4 cycle tick.
void Tick() {
  PPU::dot(); PPU::dot(); PPU::dot(); PPU::dot();
  registers.DIV++;
  if (registers.timer_enable) {
    switch (registers.clock_select) {
      case 0x0:
        if (registers.DIV % 256 == 0) {
          registers.TIMA++;
          CheckTimer();
        }
        break;
      case 0x1:
        if (registers.DIV % 4 == 0) {
          registers.TIMA++;
          CheckTimer();
        }
        break;
      case 0x2:
        if (registers.DIV % 16 == 0) {
          registers.TIMA++;
          CheckTimer();
        }
        break;
      case 0x3:
        if (registers.DIV % 64 == 0) {
          registers.TIMA++;
          CheckTimer();
        }
        break;
    }
  }
  if (serial_interrupt_counter > 0) {
    --serial_interrupt_counter;
    if (serial_interrupt_counter == 0) {
      registers.serial_if = true;
    }
  }
  remaining_cycles--;
  tick_count++;
  next_op_ready = true;
}

void InitializeRegisters() {
  // CPU registers
  registers.A = 0x01;
  registers.zf = 1;
  registers.nf = 0;
  registers.hf = 1;
  registers.cf = 1;
  registers.B = 0x00;
  registers.C = 0x13;
  registers.D = 0x00;
  registers.E = 0xD8;
  registers.H = 0x01;
  registers.L = 0x4D;
  registers.PC = 0x0100;
  registers.SP = 0xFFFE;

  // Timer registers
  registers.DIV = 0xAB;
  registers.TIMA = 0x00;
  registers.TAC = 0xF8;

  //Flag registers
  registers.IF = 0xE1;
  registers.IE = 0x00;
  registers.IME = false;
  registers.halt = false;

  reg_ind[0] = &registers.B;
  reg_ind[1] = &registers.C;
  reg_ind[2] = &registers.D;
  reg_ind[3] = &registers.E;
  reg_ind[4] = &registers.H;
  reg_ind[5] = &registers.L;
  reg_ind[6] = &registers.A;

  reg_16ind[0] = &registers.BC;
  reg_16ind[1] = &registers.DE;
  reg_16ind[2] = &registers.HL;
  reg_16ind[3] = &registers.SP;

};

uint8_t **GetRegIndex() {
  return reg_ind;
}

uint16_t **GetReg16Index() {
  return reg_16ind;
}

Registers &GetRegisters() {
  return registers;
}

bool nz() {
  return registers.zf == 0;
}

bool z() {
  return registers.zf;
}

bool nc() {
  return registers.cf == 0;
}

bool c() {
  return registers.cf;
}

template<typename T>
void performOpOnHl(T op) {
  uint8_t HL = rd8(registers.HL);
  op(registers, HL);
  wr8(registers.HL, HL);
}

template<typename T>
void handleOctalOpPattern(T op, int octal_col) {
  switch (octal_col) {
    case 0 ... 5:
      op(registers, *reg_ind[octal_col]);
      return;
    case 6: {
      performOpOnHl(op);
      return;
    }
    case 7:
      op(registers, registers.A);
      return;
    default:
      break;
  }
}

template<typename T>
void performOpOnHl(T op, int bit) {
  uint8_t HL = rd8(registers.HL);
  op(registers, HL, bit);
  wr8(registers.HL, HL);
}

template<typename T>
void handleOctalOpPattern(T op, int bit, int octal_col) {
  switch (octal_col) {
    case 0 ... 5:
      op(registers, *reg_ind[octal_col], bit);
      return;
    case 6: {
      performOpOnHl(op, bit);
      return;
    }
    case 7:
      op(registers, registers.A, bit);
      return;
    default:
      assert("false");
      break;
  }
}

void Execute_CB_Prefixed(uint8_t op_code) {
  int octal_row = op_code / 8;
  int octal_col = op_code % 8;
  switch (octal_row) {
    case 0:
      handleOctalOpPattern(RLC, octal_col);
      return;
    case 1:
      handleOctalOpPattern(RRC, octal_col);
      return;
    case 2:
      handleOctalOpPattern(RL, octal_col);
      return;
    case 3:
      handleOctalOpPattern(RR, octal_col);
      return;
    case 4:
      handleOctalOpPattern(SLA, octal_col);
      return;
    case 5:
      handleOctalOpPattern(SRA, octal_col);
      return;
    case 6:
      handleOctalOpPattern(SWAP, octal_col);
      return;
    case 7:
      handleOctalOpPattern(SRL, octal_col);
      return;
    case 0x8 ... 0xF:
      handleOctalOpPattern(BIT, octal_row - 0x8, octal_col);
      return;
    case 0x10 ... 0x17:
      handleOctalOpPattern(RES, octal_row - 0x10, octal_col);
      return;
    case 0x18 ... 0x1F:
      handleOctalOpPattern(SET, octal_row - 0x18, octal_col);
      return;
  }
}

void Execute_00_3F(uint8_t op_code) {
  switch (op_code % 8) {
    case 0:
      switch (op_code / 8) {
        case 0:
          return;
        case 1: {
          uint16_t addr = imm16();
          LD_MEM(addr, registers.SP & 0XFF);
          LD_MEM(addr + 1, (registers.SP & 0xFF00) >> 8);
          return;
        }
        case 2:
          // TODO: STOP
//          std::cerr << "was supposed to stop but didnt" << std::endl;
          return;
        case 3:
          JR(registers, (int8_t) rd8(registers.PC++));
          return;
        case 4:
          JR(registers, (int8_t) rd8(registers.PC++), nz());
          return;
        case 5:
          JR(registers, (int8_t) rd8(registers.PC++), z());
          return;
        case 6:
          JR(registers, (int8_t) rd8(registers.PC++), nc());
          return;
        case 7:
          JR(registers, (int8_t) rd8(registers.PC++), c());
          return;
        default:
          break;
      }
      break;
    case 1:
      switch (op_code / 8) {
        case 0:
          LD16(registers.BC, imm16());
          return;
        case 1:
          ADD_HL(registers, registers.BC);
          return;
        case 2:
          LD16(registers.DE, imm16());
          return;
        case 3:
          ADD_HL(registers, registers.DE);
          return;
        case 4:
          LD16(registers.HL, imm16());
          return;
        case 5:
          ADD_HL(registers, registers.HL);
          return;
        case 6:
          LD16(registers.SP, imm16());
          return;
        case 7:
          ADD_HL(registers, registers.SP);
          return;
        default:
          break;
      }
      break;
    case 2:
      switch (op_code / 8) {
        case 0:
          LD_MEM(registers.BC, registers.A);
          return;
        case 1:
          LD(registers.A, rd8(registers.BC));
          return;
        case 2:
          LD_MEM(registers.DE, registers.A);
          return;
        case 3:
          LD(registers.A, rd8(registers.DE));
          return;
        case 4:
          LD_MEM(registers.HL++, registers.A);
          return;
        case 5:
          LD(registers.A, rd8(registers.HL++));
          return;
        case 6:
          LD_MEM(registers.HL--, registers.A);
          return;
        case 7:
          LD(registers.A, rd8(registers.HL--));
          return;
        default:
          break;
      }
      break;
    case 3: {
      std::function<void(uint16_t &)> index_func = (op_code / 8) % 2 == 0 ? INC : DEC;
      index_func(*reg_16ind[(op_code / 8) / 2]);
      return;
    }
    case 4 ... 5: {
      std::function<void(Registers &, uint8_t &)> ind_func = op_code % 8 == 4 ?
                                                             INC_8 : DEC_8;
      int val = op_code / 8;
      switch (val) {
        case 0 ... 5:
          ind_func(registers, *reg_ind[val]);
          return;
        case 6: {
          uint8_t hl_val = rd8(registers.HL);
          ind_func(registers, hl_val);
          wr8(registers.HL, hl_val);
          return;
        }
        case 7:
          ind_func(registers, registers.A);
          return;
        default:
          break;
      }
      break;
    }
    case 6:
      switch (op_code / 8) {
        case 0 ... 5:
          LD(*reg_ind[op_code / 8], imm8());
          return;
        case 6:
          LD_MEM(registers.HL, imm8());
          return;
        case 7:
          LD(registers.A, imm8());
          return;
        default:
          break;
      }
      break;
    case 7:
      switch (op_code / 8) {
        case 0:
          RLC(registers, registers.A);
          registers.zf = 0;
          return;
        case 1:
          RRC(registers, registers.A);
          registers.zf = 0;
          return;
        case 2:
          RL(registers, registers.A);
          registers.zf = 0;
          return;
        case 3:
          RR(registers, registers.A);
          registers.zf = 0;
          return;
        case 4:
          DAA(registers);
          return;
        case 5:
          CPL(registers);
          return;
        case 6:
          SCF(registers);
          return;
        case 7:
          CCF(registers);
          return;
        default:
          break;
      }
  }
  std::cerr << "got a bad op code 0x" << std::hex << (int) op_code << std::endl;
}

void Execute_C0_FF(uint8_t op_code) {
  int octal_col = op_code % 8;
  int octal_row = (op_code / 8) - 24;
  switch (octal_col) {
    case 0:
      switch (octal_row) {
        case 0:
          RET(registers, nz());
          return;
        case 1:
          RET(registers, z());
          return;
        case 2:
          RET(registers, nc());
          return;
        case 3:
          RET(registers, c());
          return;
        case 4:
          LD_MEM(0xFF00 | rd8(registers.PC++), registers.A);
          return;
        case 5:
          ADD_SP(registers, (int8_t) rd8(registers.PC++));
          return;
        case 6:
          LD(registers.A, rd8(0xFF00 | rd8(registers.PC++)));
          return;
        case 7:
          LD_HL(registers, (int8_t) rd8(registers.PC++));
          return;
        default:
          break;
      }
      break;
    case 1:
      switch (octal_row) {
        case 0:
          POP_16(registers, registers.BC);
          return;
        case 1:
          RET(registers);
          return;
        case 2:
          POP_16(registers, registers.DE);
          return;
        case 3:
          RETI(registers);
          return;
        case 4:
          POP_16(registers, registers.HL);
          return;
        case 5:
          JP_HL(registers);
          return;
        case 6:
          POP_16(registers, registers.AF);
          registers.AF &= 0xFFF0;
          return;
        case 7:
          LD16(registers.SP, registers.HL);
          return;
        default:
          break;
      }
      break;
    case 2:
      switch (octal_row) {
        case 0:
          JP(registers, imm16(), nz());
          return;
        case 1:
          JP(registers, imm16(), z());
          return;
        case 2:
          JP(registers, imm16(), nc());
          return;
        case 3:
          JP(registers, imm16(), c());
          return;
        case 4:
          LD_MEM(0xFF00 | registers.C, registers.A);
          return;
        case 5:
          LD_MEM(imm16(), registers.A);
          return;
        case 6:
          LD(registers.A, rd8(0xFF00 + (uint16_t) registers.C));
          return;
        case 7:
          LD(registers.A, rd8(imm16()));
          return;
        default:
          break;
      }
      break;
    case 3:

      switch (octal_row) {
        case 0:
          JP(registers, imm16());
          return;
        case 1:
          Execute_CB_Prefixed(imm8());
          return;
        case 6:
          DI(registers);
          return;
        case 7:
          EI(registers);
          return;
        default:
          break;
      }
      break;
    case 4:
      switch (octal_row) {
        case 0:
          CALL(registers, imm16(), nz());
          return;
        case 1:
          CALL(registers, imm16(), z());
          return;
        case 2:
          CALL(registers, imm16(), nc());
          return;
        case 3:
          CALL(registers, imm16(), c());
          return;
        default:
          break;
      }
      break;
    case 5:
      switch (octal_row) {
        case 0:
          PUSH(registers, registers.BC);
          return;
        case 1:
          CALL(registers, imm16());
          return;
        case 2:
          PUSH(registers, registers.DE);
          return;
        case 4:
          PUSH(registers, registers.HL);
          return;
        case 6:
          PUSH(registers, registers.AF & 0xFFF0);
          return;
        default:
          break;
      }
      break;
    case 6:
      switch (octal_row) {
        case 0:
          ADD_A(registers, imm8());
          return;
        case 1:
          ADC(registers, imm8());
          return;
        case 2:
          SUB_A(registers, imm8());
          return;
        case 3:
          SBC_A(registers, imm8());
          return;
        case 4:
          AND_A(registers, imm8());
          return;
        case 5:
          XOR_A(registers, imm8());
          return;
        case 6:
          OR_A(registers, imm8());
          return;
        case 7:
          CP_A(registers, imm8());
          return;
        default:
          break;
      }
      break;
    case 7:
//      printf("this could be sus? %X %X\n ", registers.PC, op_code);
      RST(registers, 0x8 * octal_row);
      return;
    default:
      std::cerr << "unimplemented op code: 0x" << std::hex << (int) op_code << std::endl;
      assert(false);
      break;
  }
  std::cerr << "unimplemented op code: 0x" << std::hex << (int) op_code << std::endl;
  assert(false);
}

void HandleInterrupt() {
  registers.PC--;
  uint16_t intr_addr = 0;
  uint8_t interrupt = registers.IF & registers.IE;
  if (interrupt & 0x1) {
    intr_addr = 0x40;
    registers.IF ^= 0x1;
  } else if (interrupt & 0x2) {
    intr_addr = 0x48;
    registers.IF ^= 0x2;
  } else if (interrupt & 0x4) {
    intr_addr = 0x50;
    registers.IF ^= 0x4;
  } else if (interrupt & 0x8) {
    intr_addr = 0x58;
    registers.IF ^= 0x8;
  } else if (interrupt & 0x10) {
    intr_addr = 0x60;
    registers.IF ^= 0x10;
  }
//  printf("interrupt occurred \n");
  registers.IME = false;
  CALL(registers, intr_addr);
  Tick();
}

void RunFrame(bool debug) {
  remaining_cycles += kTotalCycles;
  while (remaining_cycles > 0) {
    ProcessInstruction(debug);
  }
}

void SetControllerState() {
  GUI::SetControllerState(registers.controller);
  if (registers.controller.buttons ^ 0xF) {
    registers.joypad_if = true;
  }
}

uint8_t getNextOp() {
  return rd8(registers.PC++);
}

void ProcessInstruction(bool debug) {
  SetControllerState();
  if ((registers.IE & registers.IF) > 0) {
    registers.halt = false;
    if (registers.IME) {
      rd8(registers.PC++);
      HandleInterrupt();
      return;
    }
  }
  if (registers.PC == 0x41CF) {
    printf("up in dis");
  }
  if (debug) {
    printf(
        "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X) %s\n",
        registers.A,
        registers.F,
        registers.B,
        registers.C,
        registers.D,
        registers.E,
        registers.H,
        registers.L,
        registers.SP,
        registers.PC,
        access<read>(registers.PC),
        access<read>(registers.PC + 1),
        access<read>(registers.PC + 2),
        access<read>(registers.PC + 3),
        GetOpString(registers).c_str());
//    if (registers.PC == next_break && registers.A == 0xDF && registers.L == 0x1C) {
//      found_break = true;
//      std::cin.ignore();
//    }
  }
  if (registers.halt) {
    Tick();
    return;
  }
  uint8_t op = getNextOp();
  switch (op) {
    case 0x00:
      break;
    case 0x01 ... 0x3F:
      Execute_00_3F(op);
      break;
    case 0x40 ... 0x45:
      LD(registers.B, *reg_ind[op - 0x40]);
      break;
    case 0x46:
      LD(registers.B, rd8(registers.HL));
      break;
    case 0x47:
      LD(registers.B, registers.A);
      break;
    case 0x48 ... 0x4D:
      LD(registers.C, *reg_ind[op - 0x48]);
      break;
    case 0x4E:
      LD(registers.C, rd8(registers.HL));
      break;
    case 0x4F:
      LD(registers.C, registers.A);
      break;
    case 0x50 ... 0x55:
      LD(registers.D, *reg_ind[op - 0x50]);
      break;
    case 0x56:
      LD(registers.D, rd8(registers.HL));
      break;
    case 0x57:
      LD(registers.D, registers.A);
      break;
    case 0x58 ... 0x5D:
      LD(registers.E, *reg_ind[op - 0x58]);
      break;
    case 0x5E:
      LD(registers.E, rd8(registers.HL));
      break;
    case 0x5F:
      LD(registers.E, registers.A);
      break;
    case 0x60 ... 0x65:
      LD(registers.H, *reg_ind[op - 0x60]);
      break;
    case 0x66:
      LD(registers.H, rd8(registers.HL));
      break;
    case 0x67:
      LD(registers.H, registers.A);
      break;
    case 0x68 ... 0x6D:
      LD(registers.L, *reg_ind[op - 0x68]);
      break;
    case 0x6E:
      LD(registers.L, rd8(registers.HL));
      break;
    case 0x6F:
      LD(registers.L, registers.A);
      break;
    case 0x70 ... 0x75:
      LD_MEM(registers.HL, *reg_ind[op - 0x70]);
      break;
    case 0x76:
      registers.halt = true;
      break;
    case 0x77:
      LD_MEM(registers.HL, registers.A);
      break;
    case 0x78 ... 0x7D:
      LD(registers.A, *reg_ind[op - 0x78]);
      break;
    case 0x7E:
      LD(registers.A, rd8(registers.HL));
      break;
    case 0X7F:
      LD(registers.A, registers.A);
      break;
    case 0x80:
      ADD_A(registers, registers.B);
      break;
    case 0x81:
      ADD_A(registers, registers.C);
      break;
    case 0x82:
      ADD_A(registers, registers.D);
      break;
    case 0x83:
      ADD_A(registers, registers.E);
      break;
    case 0x84:
      ADD_A(registers, registers.H);
      break;
    case 0x85:
      ADD_A(registers, registers.L);
      break;
    case 0x86:
      ADD_A(registers, rd8(registers.HL));
      break;
    case 0x87:
      ADD_A(registers, registers.A);
      break;
    case 0x88:
      ADC(registers, registers.B);
      break;
    case 0x89:
      ADC(registers, registers.C);
      break;
    case 0x8A:
      ADC(registers, registers.D);
      break;
    case 0x8B:
      ADC(registers, registers.E);
      break;
    case 0x8C:
      ADC(registers, registers.H);
      break;
    case 0x8D:
      ADC(registers, registers.L);
      break;
    case 0x8E:
      ADC(registers, rd8(registers.HL));
      break;
    case 0x8F:
      ADC(registers, registers.A);
      break;
    case 0x90:
      SUB_A(registers, registers.B);
      break;
    case 0x91:
      SUB_A(registers, registers.C);
      break;
    case 0x92:
      SUB_A(registers, registers.D);
      break;
    case 0x93:
      SUB_A(registers, registers.E);
      break;
    case 0x94:
      SUB_A(registers, registers.H);
      break;
    case 0x95:
      SUB_A(registers, registers.L);
      break;
    case 0x96:
      SUB_A(registers, rd8(registers.HL));
      break;
    case 0x97:
      SUB_A(registers, registers.A);
      break;
    case 0x98:
      SBC_A(registers, registers.B);
      break;
    case 0x99:
      SBC_A(registers, registers.C);
      break;
    case 0x9A:
      SBC_A(registers, registers.D);
      break;
    case 0x9B:
      SBC_A(registers, registers.E);
      break;
    case 0x9C:
      SBC_A(registers, registers.H);
      break;
    case 0x9D:
      SBC_A(registers, registers.L);
      break;
    case 0x9E:
      SBC_A(registers, rd8(registers.HL));
      break;
    case 0x9F:
      SBC_A(registers, registers.A);
      break;
    case 0xA0:
      AND_A(registers, registers.B);
      break;
    case 0xA1:
      AND_A(registers, registers.C);
      break;
    case 0xA2:
      AND_A(registers, registers.D);
      break;
    case 0xA3:
      AND_A(registers, registers.E);
      break;
    case 0xA4:
      AND_A(registers, registers.H);
      break;
    case 0xA5:
      AND_A(registers, registers.L);
      break;
    case 0xA6:
      AND_A(registers, rd8(registers.HL));
      break;
    case 0xA7:
      AND_A(registers, registers.A);
      break;
    case 0xA8:
      XOR_A(registers, registers.B);
      break;
    case 0xA9:
      XOR_A(registers, registers.C);
      break;
    case 0xAA:
      XOR_A(registers, registers.D);
      break;
    case 0xAB:
      XOR_A(registers, registers.E);
      break;
    case 0xAC:
      XOR_A(registers, registers.H);
      break;
    case 0xAD:
      XOR_A(registers, registers.L);
      break;
    case 0xAE:
      XOR_A(registers, rd8(registers.HL));
      break;
    case 0xAF:
      XOR_A(registers, registers.A);
      break;
    case 0xB0:
      OR_A(registers, registers.B);
      break;
    case 0xB1:
      OR_A(registers, registers.C);
      break;
    case 0xB2:
      OR_A(registers, registers.D);
      break;
    case 0xB3:
      OR_A(registers, registers.E);
      break;
    case 0xB4:
      OR_A(registers, registers.H);
      break;
    case 0xB5:
      OR_A(registers, registers.L);
      break;
    case 0xB6:
      OR_A(registers, rd8(registers.HL));
      break;
    case 0xB7:
      OR_A(registers, registers.A);
      break;
    case 0xB8:
      CP_A(registers, registers.B);
      break;
    case 0xB9:
      CP_A(registers, registers.C);
      break;
    case 0xBA:
      CP_A(registers, registers.D);
      break;
    case 0xBB:
      CP_A(registers, registers.E);
      break;
    case 0xBC:
      CP_A(registers, registers.H);
      break;
    case 0xBD:
      CP_A(registers, registers.L);
      break;
    case 0xBE:
      CP_A(registers, rd8(registers.HL));
      break;
    case 0xBF:
      CP_A(registers, registers.A);
      break;
    case 0xC0 ... 0xFF:
      Execute_C0_FF(op);
      break;
    default:
      std::cerr << "unimplemented op code: 0x" << std::hex << (int) op << std::endl;
      assert(false);
      break;
  }
}

}  //  namespace CPU

