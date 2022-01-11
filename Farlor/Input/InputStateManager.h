#pragma once

#include "ControllerButton.h"
#include "KeyboardButton.h"
#include "MouseButton.h"

#include "ControllerManager.h"

#include <FMath/FMath.h>

#include <vector>

#include <array>
#include <memory>

class ControllerManager;

constexpr uint32_t NumBufferedInputStates = 5;
constexpr uint32_t MaxControllerCount = 4;

namespace Farlor
{

    // Wrapper around a float value, values are valid between 0.0 and 1.0
    class UnidirectionalAbsoluteAxis
    {
    public:
        UnidirectionalAbsoluteAxis();
        UnidirectionalAbsoluteAxis(float normalizedFloatValue);
        UnidirectionalAbsoluteAxis(float currentValue, float maxValue);
        UnidirectionalAbsoluteAxis(int currentValue, int maxValue);

        float GetValue() const {
            return m_value;
        }

        void SetValueNormalized(float normalizedValue);
        void SetValueUnnormalized(float currentValue, float maxValue);
        void SetValueDiscretized(int currentValue, int maxValue);

    private:
        float m_value;
    };

    // Wrapper around a float value, values are valid between -1.0 and 1.0
    class BidirectionalAbsoluteAxis
    {
    public:
        BidirectionalAbsoluteAxis();
        BidirectionalAbsoluteAxis(float normalizedFloatValue);
        BidirectionalAbsoluteAxis(float currentValue, float maxValue);
        BidirectionalAbsoluteAxis(int currentValue, int maxValue);

        float GetValue() const {
            return m_value;
        }

        void SetValueNormalized(float normalizedValue);
        void SetValueUnnormalized(float currentValue, float maxValue);
        void SetValueDiscretized(int currentValue, int maxValue);

    private:
        float m_value;
    };

    enum ControllerType
    {
        UNKNOWN = 0,
        DUALSHOCK_4 = 1,
        XBOX_360 = 2
    };

    struct ControllerState
    {
        uint32_t buttonStates = 0x0;
        UnidirectionalAbsoluteAxis m_leftTrigger;
        UnidirectionalAbsoluteAxis m_rightTrigger;

        BidirectionalAbsoluteAxis m_leftJoystickX;
        BidirectionalAbsoluteAxis m_leftJoystickY;

        BidirectionalAbsoluteAxis m_rightJoystickX;
        BidirectionalAbsoluteAxis m_rightJoystickY;

        ControllerType m_controllerType = ControllerType::UNKNOWN;
    };

    struct ButtonState
    {
        bool endedDown = false;
        uint32_t numHalfSteps = 0;

        void Reset();
    };

    struct InputState
    {
        // Mouse state
        Farlor::Vector2 m_mouseUpdateDelta;
        ButtonState m_mouseLeft;
        ButtonState m_mouseRight;

        std::array<ButtonState, MouseButtons::NumMouseButtons> m_mouseButtonStates;

        // Keyboard State
        std::array <ButtonState, KeyboardButtons::NumKeyboardButtons> m_keyboardButtonStates;

        // Simply allow one controller atm
        std::array<ControllerState, MaxControllerCount> m_controllerStates;

        void Clear();
    };

    class InputStateManager
    {
    public:
        InputStateManager();
        ~InputStateManager();

        // This returns a structure containing all the input we need since the last frame
        const InputState& GetLatestInputState() {
            return m_inputStates[m_readInputStateIdx];
        }

        // Called once per frame
        void Tick();

        // Update Input States
        void SetKeyboardState(uint32_t keyboardButtonIndex, bool isDown);
        void SetMouseState(uint32_t mouseButtonIndex, bool isDown);

        void AddMouseDelta(int32_t deltaX, int32_t deltaY);

    private:
        void PollControllers();

    private:
        std::array<InputState, NumBufferedInputStates> m_inputStates;
        uint32_t m_writeInputStateIdx = 0;
        uint32_t m_readInputStateIdx = NumBufferedInputStates - 1;

        ControllerManager m_controllerManager;
    };
}