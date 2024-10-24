set(CMAKE_CXX_STANDARD 17)
add_library(cpu cpu.cpp cpu.h
        cartridge.cpp
        cartridge.h alu.cpp alu.h mapper.cpp)

set(SOURCE_FILES main.cpp cpu cartridge.cpp cartridge.h alu.cpp
        mapper.cpp
        mapper.h
        gui.cpp
        gui.h
        mappers/mbc1.cpp
        mappers/mbc1.h
        ppu.cpp
        ppu.h
)

set(EXECUTABLE_OUTPUT_PATH ..)
add_executable(gb_emu ${SOURCE_FILES})

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SDL_2_VERSION 2.30.8)

FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.8.tar.gz
        FIND_PACKAGE_ARGS ${SDL2_VERSION} EXACT
)

FetchContent_MakeAvailable(SDL2)


enable_testing()

add_executable(
        cpu_test cpu_test.cpp cpu alu.cpp cartridge.cpp mapper.cpp mappers/mbc1.cpp
        ppu.cpp
)

add_executable(alu_test alu.cpp cpu.cpp
        alu_test.cpp mapper.cpp cartridge.cpp mappers/mbc1.cpp
        ppu.cpp)

target_link_libraries(
        gb_emu
        SDL2::SDL2
)

target_link_libraries(
        cpu_test
        GTest::gtest_main
)
target_link_libraries(
        alu_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cpu_test)
gtest_discover_tests(alu_test)