set(CMAKE_C_FLAGS "")

if(LINUX)
    include(${CMAKE_SOURCE_DIR}/cmake/xcb.cmake)
    file(GLOB TMP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linux/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/linux/*.c")
    list(APPEND SOURCE_LIST ${TMP_LIST})
endif()

if(MACOS)
    list(APPEND LIB_LIST
        "-framework Foundation"
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework CoreGraphics"
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")

    file(GLOB TMP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/macos/*.m")
    list(APPEND SOURCE_LIST ${TMP_LIST})
endif()

if(WINDOWS)
    file(GLOB TMP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.c")
    list(APPEND SOURCE_LIST ${TMP_LIST})
endif()

set_property(GLOBAL APPEND PROPERTY GLOBAL_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set_property(GLOBAL APPEND PROPERTY GLOBAL_SOURCE_LIST ${SOURCE_LIST})
set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST ${LIB_LIST})
