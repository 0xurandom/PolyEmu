cmake_minimum_required(VERSION 3.15)

project(PolyEmu VERSION 0.1 LANGUAGES CXX)

find_package(raylib 4.5 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(polyemu)

target_sources(polyemu PRIVATE
  src/main.cpp
  src/backend/pico8/pico8.cpp
  src/frontend/frontend.cpp
)

target_link_libraries(polyemu PRIVATE raylib)
