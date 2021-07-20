set (Sources

    FEnvVar.cpp
    # FCompress.cpp
    FHash.cpp
    HashedString.cpp
    StringUtil.cpp
    Timer.cpp
    UIDGenerator.cpp
)

set (Includes

    FEnvVar.h
    # FCompress.h
    FHash.h
    HashedString.h
    StringID.h
    StringUtil.h
    Timer.h
    UIDGenerator.h
)

add_library(Utils
    ${Sources}
    ${Includes})

target_include_directories(Utils
PUBLIC
    $ENV{EXTERNAL_INCLUDES}
)

target_include_directories(Utils
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(Farlor::Utils ALIAS Utils)