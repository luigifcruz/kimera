cmake_minimum_required(VERSION 3.15)
project(v4m)

add_library(v4m SHARED
    v4m.m
    CameraAdapter.m
)

target_include_directories(sdl INTERFACE .)
target_link_libraries(v4m
    "-framework Foundation"
    "-framework AVFoundation"
)