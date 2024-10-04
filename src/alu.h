//
// Created by Brian Bonafilia on 9/28/24.
//

#ifndef GB_EMU_ALU_H
#define GB_EMU_ALU_H

#include "cpu.h"

namespace CPU {

void ADC(Registers &registers, uint8_t r);

void ADD_A(Registers& registers, uint8_t r);

void AND_A(Registers& registers, uint8_t r);

void CP_A(Registers& registers, uint8_t r);

void DEC_8(Registers& registers, uint8_t& r);

void OR_A(Registers& registers, uint8_t r);

void SBC_A(Registers& registers, uint8_t r);

void SUB_A(Registers& registers, uint8_t r);

void XOR_A(Registers& registers, uint8_t r);

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
