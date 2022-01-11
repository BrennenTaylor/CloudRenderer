#include "InputStateManager.h"

#include "ControllerManager.h"

namespace Farlor
{
    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis()
        : m_value(0.0f)
    {
    }

    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(float normalizedFloatValue)
        : m_value(normalizedFloatValue)
    {
    }

    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(float currentValue, float maxValue)
        : m_value(currentValue / maxValue)
    {
    }

    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(int currentValue, int maxValue)
        : m_value((float)currentValue / (float)maxValue)
    {
    }

    void UnidirectionalAbsoluteAxis::SetValueNormalized(float normalizedValue)
    {
        m_value = normalizedValue;
    }

    void UnidirectionalAbsoluteAxis::SetValueUnnormalized(float currentValue, float maxValue)
    {
        m_value = currentValue / maxValue;
    }

    void UnidirectionalAbsoluteAxis::SetValueDiscretized(int currentValue, int maxValue)
    {
        m_value = (float)currentValue / (float)maxValue;
    }

    // Wrapper around a float value, values are valid between -1.0 and 1.0
    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis()
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(float normalizedFloatValue)
        : m_value(normalizedFloatValue)
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(float currentValue, float maxValue)
        : m_value(currentValue / maxValue)
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(int currentValue, int maxValue)
        : m_value((float)currentValue / (float)maxValue)
    {
    }


    void BidirectionalAbsoluteAxis::SetValueNormalized(float normalizedValue)
    {
        m_value = normalizedValue;
    }

    void BidirectionalAbsoluteAxis::SetValueUnnormalized(float currentValue, float maxValue)
    {
        m_value = currentValue / maxValue;
    }

    void BidirectionalAbsoluteAxis::SetValueDiscretized(int currentValue, int maxValue)
    {
        m_value = (float)currentValue / (float)maxValue;
    }

    InputStateManager::InputStateManager()
        : m_inputStates()
        , m_upControllerManager{ std::make_unique<ControllerManager>() }
    {
    }

    InputStateManager::~InputStateManager()
    {
    }

    InputState* InputStateManager::GetFrameInput()
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        const InputState& readInputState = m_inputStates[m_readInputStateIdx];

        // Get controller input
        PollControllers();

        // Initialize current state
        for (uint32_t i = 0; i < MouseButtons::NumMouseButtons; ++i)
        {
            writeInputState.m_pMouseButtonStates[i].endedDown = readInputState.m_pMouseButtonStates[i].endedDown;
            writeInputState.m_pMouseButtonStates[i].numHalfSteps = 0;
        }

        for (uint32_t i = 0; i < KeyboardButtons::NumKeyboardButtons; ++i)
        {
            writeInputState.m_pKeyboardButtonStates[i].endedDown = readInputState.m_pKeyboardButtonStates[i].endedDown;
            writeInputState.m_pKeyboardButtonStates[i].numHalfSteps = 0;
        }

        for (uint32_t i = 0; i < MaxControllerCount; ++i)
        {
            for (uint32_t j = 0; j < ControllerButtons::NumControllerButtons; ++j)
            {
                writeInputState.m_ppControllerButtonStates[i][j].endedDown = readInputState.m_ppControllerButtonStates[i][j].endedDown;
                writeInputState.m_ppControllerButtonStates[i][j].numHalfSteps = 0;
            }
        }

        // Set start of mouse position to end of previous frame
        writeInputState.m_start = readInputState.m_end;

        m_writeInputStateIdx = (m_writeInputStateIdx + 1) % NumBufferedInputStates;
        m_readInputStateIdx = (m_readInputStateIdx + 1) % NumBufferedInputStates;
    }

    void InputStateManager::SetKeyboardState(uint32_t keyboardButtonIndex, bool isDown)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        // If we havent changed state, nothing to do
        if (writeInputState.m_pKeyboardButtonStates[keyboardButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        writeInputState.m_pKeyboardButtonStates[keyboardButtonIndex].endedDown = !writeInputState.m_pKeyboardButtonStates[keyboardButtonIndex].endedDown;
        writeInputState.m_pKeyboardButtonStates[keyboardButtonIndex].numHalfSteps++;
    }
    
    void InputStateManager::SetMouseState(uint32_t mouseButtonIndex, bool isDown)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        // If we havent changed state, nothing to do
        if (writeInputState.m_pMouseButtonStates[mouseButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        writeInputState.m_pMouseButtonStates[mouseButtonIndex].endedDown = !writeInputState.m_pMouseButtonStates[mouseButtonIndex].endedDown;
        writeInputState.m_pMouseButtonStates[mouseButtonIndex].numHalfSteps++;
    }

    void InputStateManager::PollControllers()
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
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
                if (writeInputState.m_ppControllerButtonStates[i][j].endedDown == m_upControllerManager->IsButtonDown(i, j))
                {
                    continue;
                }
                // Else, flip the state
                writeInputState.m_ppControllerButtonStates[i][j].endedDown = !writeInputState.m_ppControllerButtonStates[i][j].endedDown;
                writeInputState.m_ppControllerButtonStates[i][j].numHalfSteps++;
            }
        }
    }

    void InputStateManager::AddMouseDelta(int32_t deltaX, int32_t deltaY)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        writeInputState.m_end.x += static_cast<float>(deltaX);
        writeInputState.m_end.y += static_cast<float>(deltaY);
    }
}