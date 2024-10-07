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
  uint8_t r = 0b1111;
  RES(r, 2);
  EXPECT_EQ(r, 0b1011);
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



}

}  /// namespace