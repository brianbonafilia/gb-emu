set(CMAKE_CXX_STANDARD 17)
add_library(cpu cpu.cpp cpu.h
        cartridge.cpp
        cartridge.h alu.cpp alu.h)

set(SOURCE_FILES main.cpp cpu cartridge.cpp cartridge.h alu.cpp
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

enable_testing()

add_executable(
        cpu_test cpu_test.cpp cpu alu.cpp
)
target_link_libraries(
        cpu_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cpu_test)