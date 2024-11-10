//
// Created by Brian Bonafilia on 10/6/24.
//

#ifndef GB_EMU_SRC_GUI_H_
#define GB_EMU_SRC_GUI_H_

#include <cstdint>
#include "cpu.h"

namespace GUI {

void Init(bool debug);

void UpdateTexture(uint32_t* pixels);

void DrawDebugScreen(uint32_t* pixels);

void SetControllerState(CPU::Joypad& controller);

}  // namespace

#endif //GB_EMU_SRC_GUI_H_
