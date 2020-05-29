cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

configure_file(src/config.h.in config.h)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(EXTRA_LIBS ffmpeg sdl yaml)

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
  src/udp_socket.c
  src/socket.c
  src/display.c
  src/resampler.c
  src/router.c
)

if(UNIX AND NOT APPLE)
  add_subdirectory(lib/xcb)
  list(APPEND EXTRA_LIBS xcb)
  list(APPEND SOURCES
    src/loopback/linux/loopback.c
    src/loopback/linux/utils.c
    src/loopback/linux/v4l2.c
    src/loopback/linux/xcb.c
  )
endif()

if(APPLE)
  list(APPEND LIBS 
    "-framework Foundation"
    "-framework AVFoundation"
    "-framework CoreMedia"
    "-framework CoreVideo"
    "-framework CoreGraphics"
  )

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")

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

add_executable(kimera ${SOURCES})
target_include_directories(kimera PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(kimera ${EXTRA_LIBS} ${LIBS})
