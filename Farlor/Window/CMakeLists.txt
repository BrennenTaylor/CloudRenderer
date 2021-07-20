set (Sources
    WindowFactory.cpp

    Win32/Win32Window.cpp
)

set (Includes
    MouseButtons.h
    WindowFactory.h
    WindowEvents.h

    Win32/Win32Window.h
)

add_library(WindowFactory STATIC
    ${Sources}
    ${Includes}
)

target_include_directories(WindowFactory
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(WindowFactory
    FMath::FMath
)

add_library(Farlor::WindowFactory ALIAS WindowFactory)