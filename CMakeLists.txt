cmake_minimum_required(VERSION 3.15)
project(kimera VERSION 1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

list(APPEND SOURCES
  src/kimera.c
  src/codec/decoder.c
  src/codec/encoder.c
  src/transmitter.c
  src/receiver.c
  src/unix_socket.c
  src/tcp_socket.c
  src/display.c
  src/resampler.c
  src/loopback/loopback.h
)

add_executable(kimera MACOSX_BUNDLE ${SOURCES})

add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/sdl)

if(APPLE)
  add_subdirectory(lib/v4m)
  list(APPEND EXTRA_LIBS v4m)

  configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in
      ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
  )

  set_target_properties(kimera PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
  )
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/CourierPrime-Regular.ttf
  ${CMAKE_CURRENT_SOURCE_DIR}/build/CourierPrime-Regular.ttf
  COPYONLY
)

target_link_libraries(kimera ffmpeg sdl ${EXTRA_LIBS})
