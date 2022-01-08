#pragma once

#include "InputState.h"

#include <memory>

namespace Farlor
{
    class ControllerManager;

    class InputStateManager
    {
    public:
        InputStateManager();
        ~InputStateManager();

        // This returns a structure containing all the input we need since the last frame
        InputState* GetFrameInput();

        // Update Input States
        void SetKeyboardState(uint32_t keyboardButtonIndex, bool isDown);
        void SetMouseState(uint32_t mouseButtonIndex, bool isDown);

        void AddMouseDelta(int32_t deltaX, int32_t deltaY);

    private:
        void PollControllers();

    private:
        InputState* m_pCurrentInputState;
        InputState* m_pPreviousInputState;

        std::unique_ptr<ControllerManager> m_upControllerManager;
    };
}