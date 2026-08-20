cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PolyEmu VERSION 0.1 LANGUAGES CXX)

find_package(raylib 4.5 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(polyemu)

target_sources(polyemu PRIVATE
  src/main.cpp
  src/backend/chip8/chip8.cpp
  src/backend/intel8080/intel8080.cpp
  src/frontend/frontend.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON_C REQUIRED json-c)
target_include_directories(polyemu PRIVATE ${JSON_C_INCLUDE_DIRS})
target_link_libraries(polyemu PRIVATE ${JSON_C_LIBRARIES})

find_package(libzip REQUIRED)
target_link_libraries(polyemu PRIVATE libzip::zip)

target_include_directories(polyemu PRIVATE vendor)
add_subdirectory(vendor/nativefiledialog-extended)
target_link_libraries(polyemu PRIVATE nfd)

target_link_libraries(polyemu PRIVATE raylib)

# if(UNIX AND NOT APPLE)
    # set(APPDIR "${CMAKE_BINARY_DIR}/PolyEmu.AppDir")
    # file(WRITE ${CMAKE_BINARY_DIR}/polyemu.desktop
# "[Desktop Entry]
# Type=Application
# Name=PolyEmu
# Exec=polyemu
# Icon=polyemu
# Categories=Emulator;
# ")
#
    # file(WRITE ${CMAKE_BINARY_DIR}/AppRun
    # )
#
    # add_custom_target(appimage
      # DEPENDS polyemu
      # COMMAND ${CMAKE_COMMAND} -E make_directory ${APPDIR}/usr/bin
#
      # COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:polyemu> ${APPDIR}/usr/bin
      # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/polyemu.desktop ${APPDIR}
     # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/
    # )
