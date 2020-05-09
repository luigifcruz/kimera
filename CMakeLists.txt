cmake_minimum_required(VERSION 3.10)
project(kimera VERSION 1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

configure_file(src/config.h.in config.h)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/yaml)
add_subdirectory(lib/sdl)

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
)

if(UNIX AND NOT APPLE)
  list(APPEND SOURCES
    src/loopback/linux/loopback.h
    src/loopback/linux/loopback.c
  )
endif()

if(APPLE)
  list(APPEND LIBS 
    "-framework Foundation"
    "-framework AVFoundation"
    "-framework CoreMedia"
    "-framework CoreVideo"
  )

  list(APPEND SOURCES
    src/loopback/macos/loopback.m
    src/loopback/macos/FrameReceiver.m
    src/loopback/macos/CameraAdapter.m
  ) 
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/CourierPrime-Regular.ttf
  ${CMAKE_CURRENT_SOURCE_DIR}/build/CourierPrime-Regular.ttf
  COPYONLY
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/config.yml
  ${CMAKE_CURRENT_SOURCE_DIR}/build/config.yml
  COPYONLY
)

add_executable(kimera ${SOURCES})
target_include_directories(kimera PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(kimera ffmpeg sdl yaml ${LIBS})
