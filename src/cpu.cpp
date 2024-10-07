//
// Created by Brian Bonafilia on 9/7/24.
//
#include <cstdint>
#include "cpu.h"
#include "alu.h"
#include <iostream>
#include <functional>

namespace CPU {
namespace {
int tick_count = 0;

Registers registers;
uint8_t **reg_ind = new uint8_t *[7];
uint16_t **reg_16ind = new uint16_t *[4];
// start with 8KiB, will have to update to support GBC banking later
uint8_t *wram = new uint8_t[0x2000];
}  //  namespace

template<mode m>
uint8_t access(uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0x0000 ... 0X3FFF:
      // fixed bank ram... generally
      return 0;
    case 0x4000 ... 0x7FFF:
      // from cartridge, switchable bank often
      return 0;
    case 0x8000 ... 0x9FFF:
      // Video RAM
      return 0;
    case 0XA000 ... 0xBFFF:
      // External RAM
      return 0;
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
      // Not supposed to go here
      return 0;
    case 0xFE00 ... 0xFE9F:
      // OAM Object attribute memory
      return 0;
    case 0xFEA0 ... 0xFEFF:
      // Not usable
      return 0;
    case 0xFF00 ... 0xFF7F:
      // I/O registers
      return 0;
    case 0xFF80 ... 0xFFFE:
      // High RAM
      return 0;
    case 0xFFFF:
      // Interrupt enable register
      return 0;
  }
  return 0;
}

uint8_t imm8() {
  return rd8(registers.PC++);
}

uint16_t imm16() {
  uint16_t  val = rd16(registers.PC);
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
  Tick();
  return access<write>(addr, val);
}

void Tick() {
  tick_count++;
}

void InitializeRegisters() {
  registers.A = 0x01;
  registers.zf = 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = 0;
  registers.B = 0xFF;
  registers.C = 0x13;
  registers.D = 0x00;
  registers.E = 0xC1;
  registers.H = 0x84;
  registers.L = 0x03;
  registers.PC = 0x0100;
  registers.SP = 0xFFFE;

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

void Execute_CB_Prefixed(uint8_t op_code) {

}

void Execute_00_3F(uint8_t op_code) {
  switch (op_code % 8) {
    case 0:
      switch (op_code / 8) {
        case 0:
          return;
        case 1:
          LD_MEM(imm16(), registers.SP);
          return;
        case 2:
          // TODO: STOP
          std::cerr << "was supposed to stop but didnt" << std::endl;
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
          LD(registers.A, registers.HL--);
          return;
        default:
          break;
      }
      break;
    case 3:
    {
      std::function<void(uint16_t &)> index_func = (op_code / 8) % 2 ? DEC : INC;
      index_func(*reg_16ind[(op_code % 8) / 2]);
    }
    case 4 ... 5:
    {
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
          LD(*reg_ind[op_code / 8], rd8(registers.PC++));
          return;
        case 6:
          LD_MEM(registers.HL, rd8(registers.PC++));
          return;
        case 7:
          LD(registers.A, rd8(registers.PC++));
          return;
        default:
          break;
      }
      break;
    case 7:
      switch (op_code / 8) {
        case 0:
          RLC(registers, registers.A);
          return;
        case 1:
          RRC(registers, registers.A);
          return;
        case 2:
          RL(registers, registers.A);
          return;
        case 3:
          RR(registers, registers.A);
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
  std::cerr << "got a bad op code 0x" << std::hex << (int) op_code;
}

void Execute_C0_FF(uint8_t op_code) {
  int octal_col = op_code % 8;
  int octal_row = op_code / 8;
  switch (octal_col) {
    case 0:
      switch(octal_row) {
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
          ADD_SP(registers, (int8_t ) rd8(registers.PC++));
          return;
        case 6:
          LD(registers.A, rd8(0xFF00  | rd8(registers.PC++)));
          return;
        case 7:
          LD_HL(registers, (int8_t)rd8(registers.PC++));
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
          return;
        case 7:
          LD16(registers.SP, registers.HL);
          return;
        default:
          break;
      }
      break;
    case 3:
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
          LD(registers.A, rd8(0xFF00 + registers.C));
          return;
        case 5:
          LD(registers.A, rd8(imm16()));
          return;
        case 6:
          LD_MEM(0xFF00 + registers.C, registers.A);
          return;
        case 7:
          LD(registers.A, rd8(imm16()));
          return;
        default:
          break;
      }
      break;
    case 4:
      switch (octal_row) {
        case 0:
          JP(registers, imm16());
          return;
        case 1:
          Execute_C0_FF(imm8());
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
    case 5:
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
      RST(registers, 0x8 * octal_row);
      return;
    default:
      std::cerr << "unimplemented op code: 0x" << std::hex << (int) op_code << std::endl;
      break;
  }
}

void ProcessInstruction(bool debug) {
  if (debug) {
    std::cout << tick_count << std::endl;
  }
  uint8_t op = rd8(registers.PC++);
  switch (op) {
    case 0x00 ... 0x3F:
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
    case 0x48 ... 0x4D:
      LD(registers.C, *reg_ind[op - 0x40]);
      break;
    case 0x4E:
      LD(registers.C, rd8(registers.HL));
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
      // TODO: HALT
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
      break;

  }
}

}  //  namespace CPU

