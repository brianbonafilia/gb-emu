//
// Created by Brian Bonafilia on 9/28/24.
//

#include "alu.h"
#include "cpu.h"
#include <cstdint>
namespace CPU {

void ADC(Registers &registers, uint8_t r) {
  uint8_t A = r + registers.cf;
  registers.zf = A == 0;
  registers.cf = A == 0;
  registers.nf = 0;
  registers.hf = A & 10;
}

void ADD_A(Registers &registers, uint8_t r) {
  uint16_t A = r + registers.A;
  registers.zf = (A & 0xFF) == 0;
  registers.cf = (A >> 8) > 0;
  registers.hf = (0xF & r) + (0xF + registers.A) > 0xF;
  registers.A = 0XFF & A;
}

void AND_A(Registers &registers, uint8_t r) {
  registers.A &=  r;
  registers.zf = registers.A == 0;
  registers.nf = 0;
  registers.hf = 1;
  registers.cf = 0;
}

void CP_A(Registers& registers, uint8_t r) {
  registers.A -= r;
  registers.zf = registers.A == 0;
  registers.nf = 1;
  registers.hf = (registers.A & 0xF) < (r & 0xF);
  registers.cf = registers.A < r;
}

void DEC_8(Registers& registers, uint8_t& r) {
  r--;
  registers.zf = r == 0;
  registers.nf = 1;
  registers.hf = (r & 0xF) == 0xF;
}

void INC_8(Registers& registers, uint8_t& r) {
  r++;
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = ((r - 1) & 0xF) == 0xF;
}

void OR_A(Registers& registers, uint8_t r) {
  registers.A |= r;
  registers.zf = registers.A == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = 0;
}

void SUB_A(Registers& registers, uint8_t r) {
  uint8_t A = registers.A - r;
  registers.zf = A == 0;
  registers.nf = 1;
  registers.hf = ((registers.A & 0xF0) - (r & 0xF0) & 0xF) > 0;
  registers.cf = r > registers.A;
  registers.A = A;
}

void SBC_A(Registers& registers, uint8_t r) {
  SUB_A(registers, r + registers.cf);
}

void XOR_A(Registers& registers, uint8_t r) {
  registers.A ^= r;
  registers.zf = registers.A == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = 0;
}

void ADD_HL(Registers& registers, uint16_t r) {
  uint32_t sum = registers.HL + r;
  registers.zf = sum == 0 || sum == 0x10000;
  registers.nf = 0;
  registers.hf = (registers.HL & 0xFF) + (r & 0xFF) > 0xFF;
  registers.cf = sum > 0xFFFF;
  registers.HL = sum;
  Tick();
}

void DEC(uint16_t& r) {
  Tick();
  r--;
}

void INC(uint16_t& r) {
  Tick();
  r++;
}

void LD(uint8_t& r1, uint8_t r2) {
  r1 = r2;
}

void LD_MEM(uint16_t addr, uint8_t r) {
  wr8(addr, r);
}

void LD16(uint16_t& r1, uint16_t r2) {
  r1 = r2;
}


} // Namespace