cmake_minimum_required(VERSION 3.13)
project(kimera C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/sdl)
#add_subdirectory(src/decoder)
add_subdirectory(src/encoder)

target_link_libraries(kenc ffmpeg sdl ${EXTRA_LIBS})
