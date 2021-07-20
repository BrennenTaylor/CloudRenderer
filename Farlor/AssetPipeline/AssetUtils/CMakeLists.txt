add_library(AssetUtils
    Mesh.h
    Mesh.cpp
    parser.h
    parser.cpp
    # PBRTParser.h
    # PBRTParser.cpp
)

target_include_directories(AssetUtils
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(AssetUtils
    Math
    World
    Util
)

target_link_libraries(AssetUtils
    debug
    assimpd
)

target_link_libraries(AssetUtils
    optimized
    assimp
)