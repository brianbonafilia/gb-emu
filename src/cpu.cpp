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
uint8_t** reg_ind = new uint8_t*[10];
}  //  namespace

template <mode m>
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
      return 0;
    case 0xD000 ... 0xFDFF:
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
      // Interrupt register
      return 0;
  }
  return 0;
}

uint16_t rd16(uint16_t addr) {
  Tick();
  Tick();
  return access<read>(addr) << 8 || access<read>(addr + 1);
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
};

void Execute_00_3F(int8_t op_code) {
  if (op_code % 8 == 2) {
    switch(op_code / 8) {
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
        std::cerr << "unexpected LD val" << op_code;
        return;
    }
  }
  if (op_code % 8 == 3) {
    switch (op_code / 8) {

    }
  }
  if (op_code % 8 == 4 || op_code % 8 == 5) {
    std::function<void(Registers&, uint8_t&)> ind_funct = op_code & 8 == 4 ?
        INC_8 : DEC_8;
    int val = op_code / 8;
    switch (val) {
      case 0 ... 5:
        ind_funct(registers, *reg_ind[val]);
        return;
      case 6: {
        uint8_t hl_val = rd8(registers.HL);
        ind_funct(registers, hl_val);
        wr8(registers.HL, hl_val);
        return;
      }
      case 7:
        ind_funct(registers, registers.A);
        return;
      default:
        std::cerr << "unexpected INC8/DEC8 val" << val;
        return;
    }
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
    case 0x01:
      LD16(registers.BC, rd16(registers.PC));
      registers.PC += 2;
      break;
    case 0x02:

    case 0x03:
      INC(registers.BC);
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
    default:
      std::cerr << "unimplemented op code: 0x" << std::hex << (int) op << std::endl;
      break;

  }
}

}  //  namespace CPU

