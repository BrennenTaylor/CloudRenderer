#pragma once

#include "ControllerButton.h"
#include "KeyboardButton.h"
#include "MouseButton.h"

#include <FMath/FMath.h>

#include <unordered_map>
#include <vector>

namespace Farlor
{
    using MousePos = Vector2;

    struct ButtonState
    {
        bool endedDown;
        uint32_t numHalfSteps;
    };

    // Snapshot of a frames input
    // This is a certain size, we can pool these and save input state over time for playback... tight!
    struct InputState
    {
        // Mouse state
        MousePos m_start;
        MousePos m_end;

        ButtonState m_pMouseButtonStates[MouseButtons::NumMouseButtons];

        // Keyboard State
        ButtonState m_pKeyboardButtonStates[KeyboardButtons::NumKeyboardButtons];

        // Controller State
        ButtonState m_ppControllerButtonStates[MaxNumControllers][ControllerButtons::NumControllerButtons];
    };
}