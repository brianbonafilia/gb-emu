//
// Created by Brian Bonafilia on 9/7/24.
//
#include <iostream>
#include "cpu.h"
#include "gui.h"
#include "cartridge.h"

constexpr char kDebugFlag[] = "--debug";

int main(int argc, char* argv[]) {
  bool debug = false;
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << std::endl;
    if (std::string(argv[i]) == kDebugFlag) {
      debug = true;
      std::cout << "debug flag found" << std::endl;
    }
  }
  if (argc < 1) {
    std::cerr << "must include a rom to play :) " << std::endl;
  }
  Cartridge::load_cartridge(argv[argc - 1]);
  CPU::InitializeRegisters();
  GUI::Init();
  while (true) {
    //CPU::ProcessInstruction(debug);
  }
}
