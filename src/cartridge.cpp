//
// Created by Brian Bonafilia on 10/1/24.
//

#include <cstdint>
#include <string>
#include <iostream>
#include "mapper.h"
#include "cpu.h"

namespace Cartridge {

uint8_t *data;
Mapper* mapper;

uint8_t read(uint16_t addr) {
  return mapper->read(addr);
}

uint8_t write(uint16_t addr, uint8_t val) {
  return mapper->write(addr, val);
}

void load_cartridge(const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  if (file == nullptr) {
    std::cerr << "failed to open the cartridge ROM" << std::endl;
  }
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  data = new uint8_t[size];
  mapper = new Mapper(data);
  size_t bytes_read = fread(data, 1, size, file);
  if (bytes_read  < size) {
    std::cerr << "Failed to read from the file with error code" << std::endl;
    exit(2);
  }

  int cartride_type = data[0x147];
  int rom_size = data[0x148];
  int ram_size = data[0x149];

  std::cout << "the cartridge type is " << std::hex << cartride_type << std::endl;
  std::cout << "rom size is " << std::hex << rom_size << std::endl;
  std::cout << "ram size is " << std::hex << ram_size << std::endl;

}
}  // namespace Cartridge
