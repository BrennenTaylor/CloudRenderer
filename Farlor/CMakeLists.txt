cmake_minimum_required(VERSION 3.19)

project(Farlor)

add_library(FarlorEngine

    Input/ControllerManager.cpp
    Input/InputProcessor.cpp
    Input/InputState.cpp
    Input/InputStateManager.cpp

    Input/Buttons.h
    Input/ControllerButton.h
    Input/InputProcessor.h
    Input/InputState.h
    Input/InputStateManager.h
    Input/KeyboardButton.h
    Input/KeyCodes.h
    Input/Keys.h
    Input/MouseButton.h
)

target_include_directories(FarlorEngine
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FarlorEngine
    PUBLIC FMath::FMath
    PUBLIC xinput
)

add_subdirectory(Renderer)
add_subdirectory(Util)
add_subdirectory(Window)
add_subdirectory(World)