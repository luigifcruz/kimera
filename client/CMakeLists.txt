cmake_minimum_required(VERSION 3.13)
project(camera C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/sdl)

list(APPEND SOURCES
  src/main.c
  src/decoder.h
  src/decoder.c
  src/socket.h
  src/socket.c
  src/display.h
  src/display.c
  src/config.h
  src/utils.h
  src/loopback.h
  src/loopback.c
  src/resample.h
  src/resample.c
)

add_executable(camera ${SOURCES})
target_link_libraries(camera ffmpeg sdl ${EXTRA_LIBS})
