include(${CMAKE_SOURCE_DIR}/cmake/ffmpeg.cmake)

add_subdirectory(loopback)
add_subdirectory(transport)
#add_subdirectory(render)
add_subdirectory(codec)
add_subdirectory(client)

if(LINUX OR MACOS OR WINDOWS)
    file(GLOB TMP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
    list(APPEND SOURCE_LIST ${TMP_LIST})
endif()

set_property(GLOBAL APPEND PROPERTY GLOBAL_SOURCE_LIST ${SOURCE_LIST})
set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST ${LIB_LIST})
