# Add Linux dependencies and source list
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)

    target_include_directories(kimera PRIVATE ${XCB_INCLUDE_DIRS})
    target_link_libraries(kimera PRIVATE ${XCB_LDFLAGS})

    file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linux/*.cc")
    target_sources(kimera PRIVATE ${SOURCE_LIST})
endif()

# Add macOS dependencies and source list
if(MACOS)
    list(APPEND LIB_LIST
        "-framework Foundation"
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework CoreGraphics"
    )
    target_link_libraries(kimera PUBLIC ${LIB_LIST})

    file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/macos/*.mm")
    target_sources(kimera PRIVATE ${SOURCE_LIST})
endif()

# Add Windows dependences and source list
if(WINDOWS)
endif()
