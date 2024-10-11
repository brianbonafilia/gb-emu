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

void BIT(Registers& registers, uint8_t r, uint8_t bit);

void RES(Registers& unused, uint8_t& r, uint8_t bit);

void SET(Registers& unused, uint8_t& r, uint8_t bit);

void SWAP(Registers& registers, uint8_t& r);

void RL(Registers& registers, uint8_t& r);

void RLC(Registers& registers, uint8_t& r);

void RR(Registers& registers, uint8_t& r);

void RRC(Registers& registers, uint8_t& r);

void SLA(Registers& registers, uint8_t& r);

void SRA(Registers& registers, uint8_t& r);

void SRL(Registers& registers, uint8_t& r);

void LD(uint8_t& r1, uint8_t r2);

void LD_MEM(uint16_t addr, uint8_t r);

void LD16(uint16_t& r1, uint16_t r2);

void LD_SP_TO_MEM(Registers& registers, uint16_t addr);

void LD_HL(Registers& registers, int8_t e8);

void CALL(Registers& registers, uint16_t addr);

void CALL(Registers& registers, uint16_t addr, bool cc);

void ADD_SP(Registers& registers, int8_t e8);

void PUSH(Registers& registers, uint16_t r);

void JP(Registers& registers, uint16_t addr);

void JP_HL(Registers& registers);

void JP(Registers& registers, uint16_t addr, bool cc);

void JR(Registers& r, int8_t e8);

void JR(Registers& r, int8_t e8, bool cc);

void POP_16(Registers& registers, uint16_t& r16);

void PUSH(Registers& registers, uint16_t r16);

void RET(Registers& registers);

void RET(Registers& registers, bool cc);

void RST(Registers& registers, uint8_t vec);

void RETI(Registers& registers);

void CCF(Registers& registers);

void CPL(Registers& registers);
//
void DAA(Registers& registers);

void DI(Registers& registers);

void EI(Registers& registers);

void HALT(Registers& registers);

void SCF(Registers& registers);

void STOP(Registers& registers);

}  // namespace CPU

#endif //GB_EMU_ALU_H
