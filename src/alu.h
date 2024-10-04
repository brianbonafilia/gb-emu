//
// Created by Brian Bonafilia on 9/28/24.
//

#ifndef GB_EMU_ALU_H
#define GB_EMU_ALU_H

#include "cpu.h"

namespace CPU {

// Add the value in r8 plus carry to A
void ADC(Registers &registers, uint8_t r);

template <typename T>
void ADC(Registers &registers) {
  uint8_t v = T();
  ADC(registers, v);
}

void ADD_A(Registers& registers, uint8_t r);

template <typename T>
void ADD_A(Registers& registers) {
  uint8_t v = T();
  ADD_A(registers, v);
}

void AND_A(Registers& registers, uint8_t r);

template <typename T>
void AND_A(Registers& registers) {
  uint8_t v = T();
  AND_A(registers, v);
}

void CP_A(Registers& registers, uint8_t r);

template <typename T>
void CP_A(Registers& registers) {
  uint8_t v = T();
  CP_A(registers, v);
}

void DEC_8(Registers& registers, uint8_t& r);

template <typename T>
void DEC(Registers& registers) {
  uint8_t& v = T();
  DEC(registers, v);
}

void INC(Registers& registers, uint8_t& r);

template <typename T>
void INC(Registers& registers) {
  uint8_t& v = T();
  INC(registers, v);
}

void OR_A(Registers& registers, uint8_t r);

template <typename T>
void OR_A(Registers& registers) {
  uint8_t v = T();
  OR_A(registers, v);
}

void SBC_A(Registers& registers, uint8_t r);

template <typename T>
void SBC_A(Registers& registers) {
  uint8_t v = T();
  SBC_A(registers, v);
}

void SUB_A(Registers& registers, uint8_t r);

template <typename T>
void SUB_A(Registers& registers) {
  uint8_t v = T();
  SUB_A(registers, v);
}

void XOR_A(Registers& registers, uint8_t r);

template <typename T>
void XOR_A(Registers registers) {
  uint8_t v = T();
  XOR_A(registers, v);
}

void INC_8(Registers& registers, uint8_t& r);

void ADD_HL(Registers& registers, uint16_t r);

void DEC(uint16_t& r);

void INC(uint16_t& r);

void BIT(Registers& registers, uint8_t& r, uint8_t mask);

void RES(Registers& registers, uint8_t& r, uint8_t mask);

void SET(Registers& registers, uint8_t& r, uint8_t mask);

void SWAP(Registers& registers, uint8_t& r);

void RL(Registers& registers, uint8_t& r);

void RLA(Registers& registers);

void LD(uint8_t& r1, uint8_t r2);

void LD_MEM(uint16_t addr, uint8_t r);

void LD16(uint16_t& r1, uint16_t r2);


}  // namespace CPU

#endif //GB_EMU_ALU_H
