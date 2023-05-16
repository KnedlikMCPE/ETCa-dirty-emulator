cmake_minimum_required(VERSION 3.24)
project(ETCa_emu)

set(CMAKE_CXX_STANDARD 17)
include_directories(SDL/include)

find_package(SDL2 REQUIRED COMPONENTS SDL2)

add_executable(ETCa_emu main.cpp)

target_link_libraries(ETCa_emu PRIVATE SDL2::SDL2)
