//
// Created by Brian Bonafilia on 11/19/24.
//

#ifndef GB_EMU_SRC_DEBUG_LOG_H_
#define GB_EMU_SRC_DEBUG_LOG_H_

#include <string>
#include "../cpu.h"

std::string GetOpString(CPU::Registers registers);

#endif //GB_EMU_SRC_DEBUG_LOG_H_
