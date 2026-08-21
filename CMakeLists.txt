cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PolyEmu VERSION 0.1 LANGUAGES CXX)

find_package(raylib 4.5 REQUIRED)
find_package(Lua REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(polyemu)

target_sources(polyemu PRIVATE
  src/main.cpp
  src/backend/chip8/chip8.cpp
  src/backend/intel8080/intel8080.cpp
  src/backend/pico8/pico8.cpp
  src/backend/pico8/api/cartdata.cpp
  src/backend/pico8/api/math.cpp
  src/backend/pico8/api/memory.cpp
  src/backend/pico8/utils.cpp
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

target_include_directories(polyemu PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(polyemu PRIVATE raylib  ${LUA_LIBRARIES})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(UNIX AND NOT APPLE)
    set(APPDIR "${CMAKE_BINARY_DIR}/PolyEmu.AppDir")
    set(LINUXDEPLOY "${CMAKE_BINARY_DIR}/linuxdeploy-x86_64.AppImage")

    file(WRITE ${CMAKE_BINARY_DIR}/polyemu.desktop
"[Desktop Entry]
Type=Application
Name=PolyEmu
Exec=polyemu
Icon=polyemu
Categories=Game;Emulator;
")
    file(WRITE ${CMAKE_BINARY_DIR}/polyemu.svg "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"256\" height=\"256\"><rect width=\"256\" height=\"256\" fill=\"black\"/></svg>")


    add_custom_target(appimage
      DEPENDS polyemu

      COMMAND wget -q -c https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O ${LINUXDEPLOY}
      COMMAND chmod +x ${LINUXDEPLOY}

      COMMAND ${CMAKE_COMMAND} -E make_directory ${APPDIR}/usr/bin
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:polyemu> ${APPDIR}/usr/bin
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${APPDIR}/usr/bin/assets
      COMMAND env OUTPUT=PolyEmu-x86_64.AppImage ${LINUXDEPLOY} --appdir ${APPDIR} --executable ${APPDIR}/usr/bin/polyemu --desktop-file ${CMAKE_BINARY_DIR}/polyemu.desktop --icon-file ${CMAKE_BINARY_DIR}/polyemu.svg --output appimage      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}

      )
      endif()
