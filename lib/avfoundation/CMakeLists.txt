cmake_minimum_required(VERSION 3.15)
project(avfoundation)

add_library(avfoundation SHARED
    avfoundation.m
    CameraAdapter.m
    FrameReceiver.m
)

target_include_directories(avfoundation INTERFACE .)
target_link_libraries(avfoundation
    "-framework Foundation"
    "-framework AVFoundation"
    "-framework CoreMedia"
    "-framework CoreVideo"
    "-framework CoreImage" # remove this shit after debug
    "-framework AppKit" # remove this shit after debug
)