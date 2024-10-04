//
// Created by Brian Bonafilia on 9/10/24.
//
#include <gtest/gtest.h>
#include "cpu.h"

namespace CPU {
namespace {

// Demonstrate some basic assertions.
TEST(CpuTest, ProcessInstruction) {
  ProcessInstruction(false);
}

TEST(CpuTest, RegistersCanAccessAs8Bit) {
  Registers r {};
  r.AF = 0x1234;
  EXPECT_EQ(r.A, 0x12);
  EXPECT_EQ(r.F, 0x34);

  r.BC = 0x4567;
  EXPECT_EQ(r.B, 0x45);
  EXPECT_EQ(r.C, 0x67);

  r.DE = 0x7890;
  EXPECT_EQ(r.D, 0x78);
  EXPECT_EQ(r.E, 0x90);

  r.HL = 0x9876;
  EXPECT_EQ(r.H, 0x98);
  EXPECT_EQ(r.L, 0x76);
}

TEST(CpuTest, FlagValuesAreCorrect) {
  Registers r {};
  r.F = 0x10;
  EXPECT_EQ(r.cf, 1);
  r.F <<= 1;
  EXPECT_EQ(r.hf, 1);
  r.F <<= 1;
  EXPECT_EQ(r.nf, 1);
  r.F <<= 1;
  EXPECT_EQ(r.zf, 1);
}

TEST(CpuTest, RegIndexWorksCorrect) {
  InitializeRegisters();
  Registers& r = GetRegisters();
  r.BC = 0x1234;
  uint8_t** reg_ind = GetRegIndex();
  uint16_t** reg_16ind = GetReg16Index();
  EXPECT_EQ(*reg_ind[0], 0x12);
  EXPECT_EQ(*reg_ind[1], 0x34);
  EXPECT_EQ(*reg_16ind[0], 0x1234);
  r.DE = 0x4567;
  EXPECT_EQ(*reg_ind[2], 0x45);
  EXPECT_EQ(*reg_ind[3], 0x67);
  EXPECT_EQ(*reg_16ind[1], 0x4567);
  r.HL = 0x6789;
  EXPECT_EQ(*reg_ind[4], 0x67);
  EXPECT_EQ(*reg_ind[5], 0x89);
  EXPECT_EQ(*reg_16ind[2], 0x6789);
  r.SP = 0xFFFF;
  EXPECT_EQ(*reg_16ind[3], 0xFFFF);

}

TEST(CpuTest, ReadInstruction) {

}

}
}