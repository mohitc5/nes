cmake_minimum_required(VERSION 3.16)
project(nes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra)

find_package(SDL2 REQUIRED)

add_executable(nes
    src/main.cpp
    src/c6502.cpp
    src/ppu.cpp
    src/platform.cpp
)

target_link_libraries(nes PRIVATE SDL2::SDL2)
