#include "InputStateManager.h"

#include "ControllerManager.h"

namespace Farlor
{
    InputStateManager::InputStateManager()
        : m_pCurrentInputState{nullptr}
        , m_pPreviousInputState{nullptr}
        , m_upControllerManager{ std::make_unique<ControllerManager>() }
    {
        // Initialize to zero
        m_pCurrentInputState = new InputState;
        memset(m_pCurrentInputState, 0, sizeof(InputState));
    }

    InputStateManager::~InputStateManager()
    {
    }

    const InputState& InputStateManager::GetFrameInput() const
    {
        // Get controller input
        PollControllers();

        // Delete the previous state, only guaranteed for a frame anyways
        delete m_pPreviousInputState;
        
        m_pPreviousInputState = m_pCurrentInputState;
        m_pCurrentInputState = new InputState();

        // Initialize current state
        for (uint32_t i = 0; i < MouseButtons::NumMouseButtons; ++i)
        {
            m_pCurrentInputState->m_pMouseButtonStates[i].endedDown = m_pPreviousInputState->m_pMouseButtonStates[i].endedDown;
            m_pCurrentInputState->m_pMouseButtonStates[i].numHalfSteps = 0;
        }

        for (uint32_t i = 0; i < KeyboardButtons::NumKeyboardButtons; ++i)
        {
            m_pCurrentInputState->m_pKeyboardButtonStates[i].endedDown = m_pPreviousInputState->m_pKeyboardButtonStates[i].endedDown;
            m_pCurrentInputState->m_pKeyboardButtonStates[i].numHalfSteps = 0;
        }

        for (uint32_t i = 0; i < MaxNumControllers; ++i)
        {
            for (uint32_t j = 0; j < ControllerButtons::NumControllerButtons; ++j)
            {
                m_pCurrentInputState->m_ppControllerButtonStates[i][j].endedDown = m_pPreviousInputState->m_ppControllerButtonStates[i][j].endedDown;
                m_pCurrentInputState->m_ppControllerButtonStates[i][j].numHalfSteps = 0;
            }
        }

        // Set start of mouse position to end of previous frame
        m_pCurrentInputState->m_start = m_pPreviousInputState->m_end;

        return m_pPreviousInputState;
    }

    void InputStateManager::SetKeyboardState(uint32_t keyboardButtonIndex, bool isDown)
    {
        // If we havent changed state, nothing to do
        if (m_pCurrentInputState->m_pKeyboardButtonStates[keyboardButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        m_pCurrentInputState->m_pKeyboardButtonStates[keyboardButtonIndex].endedDown = !m_pCurrentInputState->m_pKeyboardButtonStates[keyboardButtonIndex].endedDown;
        m_pCurrentInputState->m_pKeyboardButtonStates[keyboardButtonIndex].numHalfSteps++;
    }
    
    void InputStateManager::SetMouseState(uint32_t mouseButtonIndex, bool isDown)
    {
        // If we havent changed state, nothing to do
        if (m_pCurrentInputState->m_pMouseButtonStates[mouseButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        m_pCurrentInputState->m_pMouseButtonStates[mouseButtonIndex].endedDown = !m_pCurrentInputState->m_pMouseButtonStates[mouseButtonIndex].endedDown;
        m_pCurrentInputState->m_pMouseButtonStates[mouseButtonIndex].numHalfSteps++;
    }

    void InputStateManager::PollControllers()
    {
        m_upControllerManager->PollConnections();

        for (uint32_t i = 0; i < MaxNumControllers; ++i)
        {
            m_upControllerManager->PollState(i);
            if (!m_upControllerManager->IsConnected(i))
            {
                continue;
            }

            // Loop through and set button states
            for (uint32_t j = 0; j < ControllerButtons::NumControllerButtons; ++j)
            {
                // If we havent changed state, nothing to do
                if (m_pCurrentInputState->m_ppControllerButtonStates[i][j].endedDown == m_upControllerManager->IsButtonDown(i, j))
                {
                    continue;
                }
                // Else, flip the state
                m_pCurrentInputState->m_ppControllerButtonStates[i][j].endedDown = !m_pCurrentInputState->m_ppControllerButtonStates[i][j].endedDown;
                m_pCurrentInputState->m_ppControllerButtonStates[i][j].numHalfSteps++;
            }
        }
    }

    void InputStateManager::AddMouseDelta(int32_t deltaX, int32_t deltaY)
    {
        m_pCurrentInputState->m_end.x += static_cast<float>(deltaX);
        m_pCurrentInputState->m_end.y += static_cast<float>(deltaY);
    }
}