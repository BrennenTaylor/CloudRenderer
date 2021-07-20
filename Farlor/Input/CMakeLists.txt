set (Sources

    ControllerManager.cpp
    InputStateManager.cpp
)

set (Includes

    Buttons.h
    ControllerManager.h
    InputState.h
    InputStateManager.h
    KeyboardButton.h
    MouseButton.h
)

add_library(Input
    ${Sources}
    ${Includes})

target_include_directories(Input
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Input
    PUBLIC $ENV{EXTERNAL_INCLUDES})

target_link_libraries(Input
    FMath::FMath
    xinput
)

add_library(Farlor::Input ALIAS Input)