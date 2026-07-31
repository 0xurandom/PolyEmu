cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PolyEmu VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib 4.5 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(polyemu)

target_sources(polyemu PRIVATE
  src/main.cpp
  src/backend/chip8/chip8.cpp
  src/frontend/frontend.cpp
)

target_include_directories(polyemu PRIVATE vendor)
add_subdirectory(vendor/nativefiledialog-extended)
target_link_libraries(polyemu PRIVATE nfd)

target_link_libraries(polyemu PRIVATE raylib)
