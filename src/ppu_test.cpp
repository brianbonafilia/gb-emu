//
// Created by Brian Bonafilia on 11/29/24.
//

#include "ppu.h"

#include <gtest/gtest.h>

namespace PPU {
namespace {

TEST(ColorPalette, Color) {
  Color c {};

  c.val = 0xFFFF;
  EXPECT_EQ(PPU::ToRgb888(c), 0xFFFFFF);

  c.val = 0x1F;
  EXPECT_EQ(PPU::ToRgb888(c), 0xFF0000);
}

TEST(ColorPalette, fromVal) {
  Color c {.val = 0xFFFF};
  EXPECT_EQ(c.blue(), 0x1F);
  EXPECT_EQ(c.green(), 0x1F);
  EXPECT_EQ(c.red(), 0x1F);
  EXPECT_EQ(PPU::ToRgb888(c), 0xFFFFFF);
  EXPECT_EQ(c.val, 0xFFFF);


  c.val = 0x001F;
  EXPECT_EQ(c.blue(), 0x0);
  EXPECT_EQ(c.green(), 0x0);
  EXPECT_EQ(c.red(), 0x1F);
}

}
}
