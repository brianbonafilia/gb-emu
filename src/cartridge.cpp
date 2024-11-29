//
// Created by Brian Bonafilia on 10/1/24.
//

#include <cstdint>
#include <string>
#include <iostream>
#include <utility>
#include "mapper.h"
#include "mappers/mbc1.h"
#include "mappers/mbc3.h"

namespace Cartridge {

uint8_t *data;
Mapper* mapper;
int ram_size_bytes;
std::string save_path;

uint8_t read(uint16_t addr) {
  return mapper->read(addr);
}

uint8_t write(uint16_t addr, uint8_t val) {
  return mapper->write(addr, val);
}

uint8_t get_bank(uint16_t addr) {
  return mapper->get_bank(addr);
}

void Save() {
  uint8_t* ram = mapper->get_ram();
  FILE *file = fopen(save_path.c_str(), "wb");
  if (file == nullptr) {
    std::cerr << "failed to open the save file" << std::endl;
  }

  size_t bytes_written = fwrite(ram, 1, ram_size_bytes, file);
  if (bytes_written < ram_size_bytes) {
    exit(1);
  }
  printf("I wrote to file! %s\n", save_path.c_str());
  fclose(file);
}

void SavePath(std::string path) {
  save_path = std::move(path);
  if (size_t pos = save_path.find_last_of('.'); pos != std::string::npos) {
    save_path = save_path.substr(0, pos);
  }
  save_path = save_path + ".sav";
}

void SetRamSize(int size) {
  switch (size) {
    case 3:
      ram_size_bytes = 0x8000;
      break;
    default:
      ram_size_bytes = 0;
  }
}

bool IsCgbMode() {
  return data[0x143] & 0x80;
}

uint8_t* GetRam() {
  if (ram_size_bytes == 0) {
    return nullptr;
  }
  auto* ram = new uint8_t[ram_size_bytes];

  FILE* file = fopen(save_path.c_str(), "rb");
  if (file == nullptr) {
    // return a empty ram
    return ram;
  }

  size_t bytes_read = fread(ram, 1, ram_size_bytes, file);
  if (bytes_read  < ram_size_bytes) {
    std::cerr << "Failed to read from the file with error code" << std::endl;
    exit(2);
  }
  return ram;
}

void load_cartridge(const char *file_path) {
  SavePath(std::string(file_path));
  FILE *file = fopen(file_path, "rb");
  if (file == nullptr) {
    std::cerr << "failed to open the cartridge ROM" << std::endl;
  }
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  data = new uint8_t[size];
  size_t bytes_read = fread(data, 1, size, file);
  if (bytes_read  < size) {
    std::cerr << "Failed to read from the file with error code" << std::endl;
    exit(2);
  }

  int cartride_type = data[0x147];
  int rom_size = data[0x148];
  int ram_size = data[0x149];
  SetRamSize(ram_size);

  uint8_t* ram = GetRam();

  switch (cartride_type) {
    case 0:
      mapper = new Mapper(data);
      break;
    case 1:
    case 2:
    case 3:
      mapper = new MBC1(data, ram, rom_size, ram_size);
      break;
    case 0x10:
    case 0x1b:  //TODO: make MBC 5 for this
    case 0x13:
      mapper = new MBC3(data, ram, rom_size, ram_size);
      break;
    default:
      std::cerr << "mapper type not supported: Mapper " << cartride_type << std::endl;
  }


  std::cout << "the cartridge type is " << std::hex << cartride_type << std::endl;
  std::cout << "rom size is " << std::hex << rom_size << std::endl;
  std::cout << "ram size is " << std::hex << ram_size << std::endl;

}
}  // namespace Cartridge
