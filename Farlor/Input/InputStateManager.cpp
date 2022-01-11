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

    void ButtonState::Reset()
    {
        endedDown = false;
        numHalfSteps = 0;
    }

    void InputState::Clear()
    {
        m_mouseUpdateDelta = Farlor::Vector2(0.0f, 0.0f);

        for (auto& mouseButton : m_mouseButtonStates)
        {
            mouseButton.Reset();
        }

        for (auto& key : m_keyboardButtonStates)
        {
            key.Reset();
        }
    }

    InputStateManager::InputStateManager()
        : m_inputStates()
        , m_controllerManager()
    {
    }

    InputStateManager::~InputStateManager()
    {
    }

    void InputStateManager::Tick()
    {
        // Rotate to the next input state
        m_writeInputStateIdx = (m_writeInputStateIdx + 1) % NumBufferedInputStates;
        m_readInputStateIdx = (m_readInputStateIdx + 1) % NumBufferedInputStates;

        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        writeInputState.Clear();

        const InputState& readInputState = m_inputStates[m_readInputStateIdx];

        // Get controller input
        PollControllers();

        // Initialize current state
        for (uint32_t i = 0; i < MouseButtons::NumMouseButtons; ++i)
        {
            writeInputState.m_mouseButtonStates[i].endedDown = readInputState.m_mouseButtonStates[i].endedDown;
            writeInputState.m_mouseButtonStates[i].numHalfSteps = 0;
        }

        for (uint32_t i = 0; i < KeyboardButtons::NumKeyboardButtons; ++i)
        {
            writeInputState.m_keyboardButtonStates[i].endedDown = readInputState.m_keyboardButtonStates[i].endedDown;
            writeInputState.m_keyboardButtonStates[i].numHalfSteps = 0;
        }

        // TODO: Update this
        //for (uint32_t i = 0; i < MaxControllerCount; ++i)
        //{
        //    for (uint32_t j = 0; j < ControllerButtons::NumControllerButtons; ++j)
        //    {
        //        writeInputState.m_ppControllerButtonStates[i][j].endedDown = readInputState.m_ppControllerButtonStates[i][j].endedDown;
        //        writeInputState.m_ppControllerButtonStates[i][j].numHalfSteps = 0;
        //    }
        //}
    }

    void InputStateManager::SetKeyboardState(uint32_t keyboardButtonIndex, bool isDown)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        // If we havent changed state, nothing to do
        if (writeInputState.m_keyboardButtonStates[keyboardButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        writeInputState.m_keyboardButtonStates[keyboardButtonIndex].endedDown = !writeInputState.m_keyboardButtonStates[keyboardButtonIndex].endedDown;
        writeInputState.m_keyboardButtonStates[keyboardButtonIndex].numHalfSteps++;
    }
    
    void InputStateManager::SetMouseState(uint32_t mouseButtonIndex, bool isDown)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        // If we havent changed state, nothing to do
        if (writeInputState.m_mouseButtonStates[mouseButtonIndex].endedDown == isDown)
        {
            return;
        }
        // Else, flip the state
        writeInputState.m_mouseButtonStates[mouseButtonIndex].endedDown = !writeInputState.m_mouseButtonStates[mouseButtonIndex].endedDown;
        writeInputState.m_mouseButtonStates[mouseButtonIndex].numHalfSteps++;
    }

    void InputStateManager::PollControllers()
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        m_controllerManager.Poll();

        for (uint32_t i = 0; i < MaxNumControllers; ++i)
        {
            if (!m_controllerManager.IsConnected(i))
            {
                continue;
            }

            // TODO: Fix this
            // Loop through and set button states
            //for (uint32_t j = 0; j < ControllerButtons::NumControllerButtons; ++j)
            //{
            //    // If we havent changed state, nothing to do
            //    if (writeInputState.m_ppControllerButtonStates[i][j].endedDown == m_controllerManager.IsButtonDown(i, j))
            //    {
            //        continue;
            //    }
            //    // Else, flip the state
            //    writeInputState.m_ppControllerButtonStates[i][j].endedDown = !writeInputState.m_ppControllerButtonStates[i][j].endedDown;
            //    writeInputState.m_ppControllerButtonStates[i][j].numHalfSteps++;
            //}
        }
    }

    void InputStateManager::AddMouseDelta(int32_t deltaX, int32_t deltaY)
    {
        InputState& writeInputState = m_inputStates[m_writeInputStateIdx];
        writeInputState.m_mouseUpdateDelta += Farlor::Vector2(static_cast<float>(deltaX), static_cast<float>(deltaY));
    }
}