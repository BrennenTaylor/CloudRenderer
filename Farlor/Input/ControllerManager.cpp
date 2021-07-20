#include "ControllerManager.h"

using namespace std;

namespace Farlor
{
    ControllerManager::ControllerManager()
        : m_connectionStatus{ false, false, false, false }
        , m_initialized{ false, false, false, false }
        , m_polledStates{ 0, 0, 0, 0 }
    {
    }

    ControllerManager::~ControllerManager()
    {
        SetVibration(0, 0.0f, 0.0f);
        SetVibration(1, 0.0f, 0.0f);
        SetVibration(2, 0.0f, 0.0f);
        SetVibration(3, 0.0f, 0.0f);
    }

    void ControllerManager::PollConnections()
    {
        for (uint32_t controllerNum = 0; controllerNum < XUSER_MAX_COUNT; controllerNum++)
        {
            XINPUT_STATE state = { 0 };
            auto result = XInputGetState(controllerNum, &state);
            if (result == ERROR_SUCCESS)
            {
                m_connectionStatus[controllerNum] = true;
            }
            else
            {
                m_connectionStatus[controllerNum] = false;
            }
        }
    }

    void ControllerManager::PollState(uint32_t index)
    {
        DWORD result;
        XINPUT_STATE state = { 0 };
        result = XInputGetState(index, &state);

        if (result != ERROR_SUCCESS)
        {
            m_connectionStatus[index] = false;
            return;
        }

        m_polledStates[index] = state;

        /*
        // Left Stick
        float LX = state.Gamepad.sThumbLX;
        float LY = state.Gamepad.sThumbLY;

        float magnitudeLH = sqrt(LX*LX + LY*LY);

        float normalizedLX = LX / magnitudeLH;
        float normalizedLY = LY / magnitudeLH;

        float normalizedMagnitudeLH;

        if (magnitudeLH > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            if (magnitudeLH > 32767)
                magnitudeLH = 32767;

            magnitudeLH -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

            // Optionally normailze the magnitudeLH with respect to expected range
            // gets 0.0f - 1.0f
            normalizedMagnitudeLH = magnitudeLH / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        }
        else
        {
            magnitudeLH = 0.0f;
            normalizedMagnitudeLH = 0.0f;
            normalizedLX = 0.0f;
            normalizedLY = 0.0f;
        }

        // Right Stick
        float RX = state.Gamepad.sThumbRX;
        float RY = state.Gamepad.sThumbRY;

        float magnitudeRH = sqrt(RX*RX + RY*RY);

        float normalizedRX = RX / magnitudeRH;
        float normalizedRY = RY / magnitudeRH;

        float normalizedMagnitudeRH;

        if (magnitudeRH > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        {
            if (magnitudeRH > 32767)
                magnitudeRH = 32767;

            magnitudeRH -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

            // Optionally normailze the magnitudeRH with respect to expected range
            // gets 0.0f - 1.0f
            normalizedMagnitudeRH = magnitudeRH / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        }
        else
        {
            magnitudeRH = 0.0f;
            normalizedMagnitudeRH = 0.0f;
            normalizedRX = 0.0f;
            normalizedRY = 0.0f;
        }
        */
    }

    bool ControllerManager::IsConnected(uint32_t controllerIndex) const
    {
        if (controllerIndex >= XUSER_MAX_COUNT || controllerIndex < 0)
        {
            // TODO: Log that controller is not in range
            return false;
        }

        return m_connectionStatus[controllerIndex];
    }

    void ControllerManager::SetVibration(uint32_t controllerIndex, float leftPercent, float rightPercent)
    {
        if (controllerIndex >= XUSER_MAX_COUNT)
        {
            // TODO: Log that vibrartion cant be sent
            return;
        }

        if (!IsConnected(controllerIndex))
        {
            // TODO: Log that controller isnt connected
            return;
        }

        // Message input to a correct range
        leftPercent = max(leftPercent, 0.0f);
        leftPercent = min(leftPercent, 1.0f);
        rightPercent = max(leftPercent, 0.0f);
        rightPercent = min(leftPercent, 1.0f);

        // Set vibration state
        const float maxVibrate = 65535;
        // https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinput_vibration(v=vs.85).aspx
        XINPUT_VIBRATION vibrationState = { 0 };
        vibrationState.wLeftMotorSpeed = static_cast<uint32_t>(leftPercent * maxVibrate);
        vibrationState.wRightMotorSpeed = static_cast<uint32_t>(rightPercent * maxVibrate);
        auto result = XInputSetState(controllerIndex, &vibrationState);
        if (result != ERROR_SUCCESS)
        {
            // TODO: Log error
        }
    }

    bool ControllerManager::IsButtonDown(uint32_t index, uint32_t controllerButtonIndex) const
    {
        if (!m_connectionStatus[index])
        {
            // TODO: Log that the state is invalid
            return false;
        }

        switch (controllerButtonIndex)
        {
        
        case ControllerButtons::DPadUp:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ? true : false;
        } break;

        case ControllerButtons::DPadDown:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ? true : false;
        } break;

        case ControllerButtons::DPadLeft:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ? true : false;
        } break;

        case ControllerButtons::DPadRight:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ? true : false;
        } break;

        case ControllerButtons::Start:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_START ? true : false;
        } break;

        case ControllerButtons::Select:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_BACK ? true : false;
        } break;

        case ControllerButtons::LeftThumb:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ? true : false;
        } break;

        case ControllerButtons::RightThumb:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? true : false;
        } break;

        case ControllerButtons::LeftBumper:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ? true : false;
        } break;

        case ControllerButtons::RightBumper:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ? true : false;
        } break;

        case ControllerButtons::A:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_A ? true : false;
        } break;

        case ControllerButtons::B:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_B ? true : false;
        } break;
        
        case ControllerButtons::X:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_X ? true : false;
        } break;

        case ControllerButtons::Y:
        {
            return m_polledStates[index].Gamepad.wButtons & XINPUT_GAMEPAD_Y ? true : false;
        } break;

        default:
        {
            // Log unsuported button case
            return false;
        } break;
        }
    }
}
