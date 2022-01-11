#pragma once

#include "ControllerButton.h"
#include "KeyboardButton.h"
#include "MouseButton.h"

#include <FMath/FMath.h>

#include <bitset>
#include <unordered_map>
#include <vector>

namespace Farlor
{
    const uint32_t MaxControllerCount = 4;

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
    };

    struct InputState
    {
        // Mouse state
        Farlor::Vector2 m_mouseUpdateDelta;
        ButtonState m_mouseLeft;
        ButtonState m_mouseRight;

        ButtonState m_pMouseButtonStates[MouseButtons::NumMouseButtons];

        // Keyboard State
        ButtonState m_pKeyboardButtonStates[KeyboardButtons::NumKeyboardButtons];

        // Simply allow one controller atm
        std::array<ControllerState, MaxControllerCount> m_controllerStates;
    };
}