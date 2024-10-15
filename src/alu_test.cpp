//
// Created by Brian Bonafilia on 10/4/24.
//

#include <gtest/gtest.h>
#include "alu.h"

namespace CPU {

TEST(ALU, CarryTest) {
  uint8_t v = 0b11000000;
  Registers r {};
  RL(r, v);
  EXPECT_TRUE(r.cf);
  EXPECT_EQ(v, 0b10000000);

  RL(r, v);
  EXPECT_EQ(v, 0b1);

  r.cf = 0;
  RR(r, v);
  EXPECT_EQ(v, 0);
  EXPECT_TRUE(r.cf);
  RR(r, v);
  EXPECT_EQ(v, 0x80);
  EXPECT_FALSE(r.cf);
}

TEST(ALU, ResZerosOutVal) {
  Registers regs{};
  uint8_t r = 0b1111;
  RES(regs, r, 2);
  EXPECT_EQ(r, 0b1011);
}

TEST(ALU, JrSubWorks) {
  Registers r { .PC = 100};
  JR(r, -10);
  EXPECT_EQ(r.PC, 90);
}

TEST(ALU, SwapWorksAsExpected) {
  Registers r {};
  r.F = 0xFF;
  r.H = 0x12;
  SWAP(r, r.H);
  EXPECT_EQ(r.H, 0x21);
}

TEST(ALU, AddSignedToSp) {
  Registers r {};
  r.SP = 0xFFFE;
  int8_t e8 = -127;

  ADD_SP(r, e8);
  EXPECT_EQ(r.SP, 0xFF7F);

  e8 = 127;
  ADD_SP(r, e8);
  EXPECT_EQ(r.SP, 0xFFFE);
}

TEST(ALU, DAA) {
  Registers r { .A = 0x23};
  DAA(r);
  EXPECT_EQ(r.A, 0x23);
  EXPECT_FALSE(r.cf);

  r.A = 0xA3;
  DAA(r);
  EXPECT_EQ(r.A, 0x3);
  EXPECT_TRUE(r.cf);

  r.cf = 0;
  r.A = 0x3B;
  DAA(r);
  EXPECT_EQ(r.A, 0x41);

  r.A = 0x45;
  ADD_A(r, 0x38);
  DAA(r);
  EXPECT_EQ(r.A, 0x83);
  SUB_A(r, 0x38);
  EXPECT_EQ(r.A, 0x4B);
  DAA(r);
  EXPECT_EQ(r.A, 0x45);

  r.hf = 0; r.cf = 0;
  r.A = 0x49;
  ADD_A(r, 0x19);
  EXPECT_EQ(r.A, 0x62);
  DAA(r);
  EXPECT_EQ(r.A, 0x68);

  r.F = 0x20;
  r.A = 0x90;
  DAA(r);
  EXPECT_EQ(r.A, 0x96);
}

TEST(ALU, INC) {
  Registers r {};
  r.D = 0x0;
  r.E = 0xC1;

  INC(r.DE);
  EXPECT_EQ(r.D, 0);
  EXPECT_EQ(r.E, 0xC2);

  DEC(r.DE);
  EXPECT_EQ(r.D, 0);
  EXPECT_EQ(r.E, 0xC1);
}

TEST(ALU, CallRet) {
  Registers r {.SP = 0xFFFE};
  CALL(r, 0xD000);
  ASSERT_EQ(r.PC++, 0xD000);
  RET(r);
  ASSERT_EQ(r.SP, 0xFFFE);
}

TEST(ALU, ADC) {
  Registers r {.A = 0xFF};
  ADC(r, 0x1);
  EXPECT_EQ(r.F , 0xB0);
  EXPECT_EQ(r.A, 0);
  EXPECT_TRUE(r.hf);

  r.A = 0x10;
  r.F = 0;
  ADC(r, 0xFF);
  EXPECT_EQ(r.A, 0x0F);
  EXPECT_EQ(r.F, 0x10);
}

TEST(ALU, SBC) {
  Registers r {.A = 00, .cf = 1};
  SBC_A(r, 0x0F);

  EXPECT_EQ(r.A, 0xF0);
  EXPECT_EQ(r.F, 0x70);
}

TEST(ALU, AddSPe8) {
  Registers r {.SP = 0xDEFB};
  ADD_SP(r, (int8_t) 0xFF);
  ASSERT_EQ(r.SP, 0xDEFA);
  ASSERT_EQ(r.F, 0x30);
}

TEST(ALU, Add) {
  Registers r{.A = 0xC1, .F = 0x05};
  SUB_A(r, 0xFC);
  EXPECT_EQ(r.A, 0xBC);
  EXPECT_EQ(r.F, 0x40);
}


}  /// namespace