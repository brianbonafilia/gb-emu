//
// Created by Brian Bonafilia on 11/19/24.
//

#include "../cpu.h"
#include <string>

std::string n16(CPU::Registers registers) {
  uint16_t  val = (CPU::access<CPU::read>(registers.PC + 2) << 8) | CPU::access<CPU::read>(registers.PC + 1);
  char hex[7];
  snprintf(hex, 7, "Ox%X", val);
  return hex;
}

std::string n8(CPU::Registers registers) {
  uint8_t  val = CPU::access<CPU::read>(registers.PC + 1);
  char hex[5];
  snprintf(hex, 5, "Ox%X", val);
  return hex;
}

std::string e8(CPU::Registers& registers) {
  int8_t val = (int8_t ) CPU::access<CPU::read>(registers.PC + 1);
  return std::to_string(val);
}

std::string GetOpString(CPU::Registers registers) {
  uint8_t op =  CPU::access<CPU::read>(registers.PC);
  switch (op) {
    case 0x0:
      return "NOP";
    case 0x1:
      return "LD BC, " + n16(registers);
    case 0x2:
      return "LD [BC], A";
    case 0x3:
      return "INC BC";
    case 0x4:
      return "INC B";
    case 0x5:
      return "DEC B";
    case 0x6:
      return "LD B, n8";
    case 0x7:
      return "RLCA";
    case 0x8:
      return "LD [" + n16(registers) + "], SP";
    case 0x9:
      return "ADD HL, BC";
    case 0xA:
      return "LD A, [BC]";
    case 0xB:
      return "DEC BC";
    case 0xC:
      return "INC C";
    case 0xD:
      return "DEC C";
    case 0xE:
      return "LD C, n8";
    case 0xF:
      return "RRCA";
    case 0x10:
      return "STOP n8";
    case 0x11:
      return "LD DE, " + n16(registers);
    case 0x12:
      return "LD [DE], " + n16(registers);
    case 0x13:
      return "INC DE";
    case 0x18:
      return "JR " + e8(registers);
    case 0x20:
      return "JR NZ, e8";
    case 0x21:
      return "LD HL, " + n16(registers);
    case 0x28:
      return "JR Z, n8";
    case 0x2A:
      return "LD A, [HL+]";
    case 0x32:
      return "LD [HL-], A";
    case 0x33:
      return "INC SP";
    case 0x38:
      return "JR C, e8";
    case 0x3B:
      return "DEC SP";
    case 0x3C:
      return "INC A";
    case 0x3E:
      return "LD A, n8";
    case 0x41:
      return "LD B, C";
    case 0x44:
      return "LD B, H";
    case 0x47:
      return "LD B, A";
    case 0x4F:
      return "LD C, A";
    case 0x60:
      return "LD H, B";
    case 0x69:
      return "LD L, C";
    case 0x70:
      return "LD [HL], B";
    case 0x77:
      return "LD [HL], A";
    case 0x7E:
      return "LD A, [HL]";
    case 0x87:
      return "ADD A, A";
    case 0x90:
      return "SUB A, B";
    case 0x91:
      return "SUB A, C";
    case 0xA7:
      return "AND A, A";
    case 0xAF:
      return "XOR A, A";
    case 0xB0:
      return "OR A, B";
    case 0xC0:
      return "RET NZ";
    case 0xC1:
      return "POP BC";
    case 0xC2:
      return "JP NZ " + n16(registers);
    case 0xC5:
      return "PUSH BC";
    case 0xC8:
      return "RET Z";
    case 0xC9:
      return "RET";
    case 0xCB:
      return "PREFIX";
    case 0xCD:
      return "CALL " + n16(registers);
    case 0xD1:
      return "POP DE";
    case 0xD5:
      return "PUSH DE";
    case 0xD9:
      return "RETI";
    case 0xE0:
      return "LDH [a8], A";
    case 0xE1:
      return "POP HL";
    case 0xE5:
      return "PUSH HL";
    case 0xE6:
      return "AND A, n8";
    case 0xEA:
      return "LD [" + n16(registers) + "], A";
    case 0xF0:
      return "LDH A, [a8]";
    case 0xF1:
      return "POP AF";
    case 0xF5:
      return "PUSH AF";
    case 0xF6:
      return "OR A, n8";
    case 0xFE:
      return "CP A, n8";
    case 0xFA:
      return "LD A, [" + n16(registers) + "]";
    default:
      return "";
  }
}
