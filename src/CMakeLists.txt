set(LIBS ffmpeg sdl yaml)

configure_file(
  "${kimera_SOURCE_DIR}/include/kimera/state.h.in"
  "${kimera_SOURCE_DIR}/include/kimera/state.h"
)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/include/kimera/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/src/*.c")

if(UNIX AND NOT APPLE)
  list(APPEND LIBS xcb)

  file(GLOB APPLE_HEADER_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/include/kimera/loopback/linux/*.h")
  list(APPEND HEADER_LIST ${APPLE_HEADER_LIST})

  file(GLOB APPLE_SOURCE_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/src/loopback/linux/*.c")
  list(APPEND SOURCE_LIST ${APPLE_SOURCE_LIST})
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

  file(GLOB APPLE_HEADER_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/include/kimera/loopback/macos/*.h")
  list(APPEND HEADER_LIST ${APPLE_HEADER_LIST})

  file(GLOB APPLE_SOURCE_LIST CONFIGURE_DEPENDS "${kimera_SOURCE_DIR}/src/loopback/macos/*.m")
  list(APPEND SOURCE_LIST ${APPLE_SOURCE_LIST})
endif()

add_library(kimera ${SOURCE_LIST} ${HEADER_LIST})

target_link_libraries(kimera ${LIBS})

target_include_directories(kimera PUBLIC ../include)