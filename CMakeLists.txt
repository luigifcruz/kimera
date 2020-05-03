cmake_minimum_required(VERSION 3.13)
project(kimera C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(lib/ffmpeg)
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
  src/resample.c
)

add_executable(kimera ${SOURCES})
target_link_libraries(kimera ffmpeg sdl ${EXTRA_LIBS})
