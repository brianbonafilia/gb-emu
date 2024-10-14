//
// Created by Brian Bonafilia on 9/28/24.
//

#include "alu.h"
#include "cpu.h"
#include <cstdint>
#include <iostream>
namespace CPU {

void ADC(Registers &registers, uint8_t r) {
  uint8_t A = registers.A + r + registers.cf;
  registers.zf = A == 0;
  registers.nf = 0;
  registers.hf = (registers.A & 0xF) + (r & 0xF) + registers.cf > 0xF;
  registers.cf = registers.A + r + registers.cf > 0xFF;
  registers.A = A;
}

void ADD_A(Registers &registers, uint8_t r) {
  uint16_t A = r + registers.A;
  registers.zf = (A & 0xFF) == 0;
  registers.cf = (A >> 8) > 0;
  registers.nf = 0;
  registers.hf = (0xF & r) + (0xF & registers.A) > 0xF;
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
  uint8_t A = registers.A - r;
  registers.zf = A == 0;
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
  registers.hf = (registers.A & 0xF) < (r & 0xF);
  registers.cf = r > registers.A;
  registers.A = A;
}

void SBC_A(Registers& registers, uint8_t r) {
  uint8_t A = registers.A - (r + registers.cf);
  registers.zf = A == 0;
  registers.nf = 1;
  registers.hf = (registers.A & 0xF) < (r & 0xF) + registers.cf;
  registers.cf = r + registers.cf > registers.A;
  registers.A = A;}

void XOR_A(Registers& registers, uint8_t r) {
  registers.A ^= r;
  registers.zf = registers.A == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = 0;
}

void ADD_HL(Registers& registers, uint16_t r) {
  uint32_t sum = registers.HL + r;
  registers.nf = 0;
  registers.hf = (registers.HL & 0xFFF) + (r & 0xFFF) > 0xFFF;
  registers.cf = sum > 0xFFFF;
  registers.HL = sum & 0xFFFF;
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

void BIT(Registers& registers, uint8_t r, uint8_t bit) {
  registers.zf = (r & (1 <<  bit)) == 0;
  registers.nf = 0;
  registers.hf = 1;
}

void RES(Registers& unused, uint8_t& r, uint8_t bit) {
  r &= ~(1 << bit);
}

void SET(Registers& unused, uint8_t& r, uint8_t bit) {
  r |= 1 << bit;
}

void SWAP(Registers& registers, uint8_t& r) {
  r = ((r & 0x0F) << 4) | ((r & 0xF0) >> 4);
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = 0;
}

void RL(Registers& registers, uint8_t& r) {
  bool carry = r & 0x80;
  r = (r << 1) + registers.cf;
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = carry > 0;
}

void RR(Registers& registers, uint8_t& r) {
  bool carry = r & 1;
  r = (r >> 1) + (registers.cf << 7);
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = carry;
}

void RLC(Registers& registers, uint8_t& r) {
  bool carry = (r & 0x80) > 0;
  r = (r << 1) + carry;
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = carry;
}

void RRC(Registers& registers, uint8_t& r) {
  bool carry = r & 1;
  r = (r >> 1) + (carry << 7);
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
  registers.cf = carry;
}

void SLA(Registers& registers, uint8_t& r) {
  registers.cf = (r & 0x80) > 0;
  r <<= 1;
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
}

void SRA(Registers& registers, uint8_t& r) {
  registers.cf = r & 1;
  r = (r & 0x80) | (r >> 1);
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
}

void SRL(Registers& registers, uint8_t& r) {
  registers.cf = r & 1;
  r >>= 1;
  registers.zf = r == 0;
  registers.nf = 0;
  registers.hf = 0;
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

void LD_SP_TO_MEM(Registers& registers, uint16_t addr) {
  wr8(addr++, registers.SP & 0xFF);
  wr8(addr, registers.SP >> 8);
}

void LD_HL(Registers& registers, int8_t e8) {
  registers.cf = (registers.SP & 0xFF) + ((uint8_t) e8 & 0xFF) > 0xFF;
  registers.hf = (registers.SP & 0xF) + ((uint8_t) e8 & 0xF) > 0xF;
  if (e8 < 0) {
    registers.HL = registers.SP - (uint8_t)(~(e8) + 1);
  } else {
    registers.HL = registers.SP + (uint8_t) e8;
  }
  registers.zf = 0;
  registers.nf = 0;
  Tick();
}

void ADD_SP(Registers& registers, int8_t e8) {
  registers.cf = (registers.SP & 0xFF) + ((uint8_t) e8 & 0xFF) > 0xFF;
  registers.hf = (registers.SP & 0xF) + ((uint8_t) e8 & 0xF) > 0xF;
  if (e8 < 0) {
    registers.SP -= (uint8_t)(~(e8) + 1);
  } else {
    registers.SP += (uint8_t) e8;
  }
  Tick(); Tick();
  registers.zf = 0;
  registers.nf = 0;
}

void JR(Registers& registers, int8_t e8) {
  Tick();
  if (e8 < 0) {
    registers.PC -= (uint8_t)(~e8 + 1);
  }
  else {
    registers.PC += e8;
  }
}

void JR(Registers& registers, int8_t e8, bool cc) {
  if (cc) {
    JR(registers, e8);
  }
}

void JP(Registers& registers, uint16_t addr) {
  registers.PC = addr;
  Tick();
}

void JP(Registers& registers, uint16_t addr, bool cc) {
  if (cc) {
    JP(registers, addr);
  }
}



void JP_HL(Registers& registers) {
  registers.PC = registers.HL;
}

void POP_16(Registers& registers, uint16_t& r16) {
  r16 = rd16(registers.SP);
  registers.SP += 2;
}

void PUSH(Registers& registers, uint16_t r16) {
  wr8(--registers.SP ,(r16 & 0xFF00) >> 8);
  wr8(--registers.SP, r16 & 0xFF);
  Tick();
}

void CALL(Registers& registers, uint16_t addr) {
  PUSH(registers, registers.PC);
  registers.PC = addr;
}

void CALL(Registers& registers, uint16_t addr, bool cc) {
  if (cc) {
    CALL(registers, addr);
  }
}

void RST(Registers& registers, uint8_t vec) {
  wr8(--registers.SP, (registers.PC & 0xFF00) >> 8);
  wr8(--registers.SP, registers.PC & 0xFF);
  registers.PC = vec;
}

void RET(Registers& registers) {
  POP_16(registers, registers.PC);
  Tick();
}

void RET(Registers& registers, bool cc) {
  if (cc) {
    RET(registers);
  }
}

void RETI(Registers& registers) {
  registers.IME = true;
  RET(registers);
}

void CCF(Registers& registers) {
  registers.cf = ~registers.cf;
  registers.nf = 0;
  registers.hf = 0;
}

void CPL(Registers& registers) {
  registers.A = ~registers.A;
  registers.nf = 1;
  registers.hf = 1;
}

void DAA(Registers& registers) {
  uint8_t low_nibble = registers.A & 0xF;
  uint8_t high_nibble = (registers.A & 0XF0) >> 4;
  if (registers.nf == 0) {
    if (low_nibble > 9) {
      high_nibble++;
      registers.hf = 1;
    }
    if (registers.hf) {
      registers.A += 0x6;
    }
    if (high_nibble > 9 || registers.cf) {
      registers.A += 0x60;
      registers.cf = 1;
    }
  } else {
    if (registers.cf)   {
      registers.A -= 0x60;
    }
    if (registers.hf) {
      registers.A -= 0x6;
    }
  }

  registers.zf = registers.A == 0;
  registers.hf = 0;
}

void DI(Registers& registers) {
  registers.IME = false;
}

void EI(Registers& registers) {
  registers.IME = true;
}

void HALT(Registers& registers) {
  // TODO: will do later
}

void SCF(Registers& registers) {
  registers.cf = 1;
  registers.nf = 0;
  registers.hf = 0;
}

void STOP(Registers& registers) {

}

} // Namespace