#pragma once

#include "ControllerButton.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <XInput.h>

namespace Farlor
{
    class ControllerManager
    {
    public:
        ControllerManager();
        ~ControllerManager();

        // Polls every controller state and checks if any have connected or disconnected
        void PollConnections();
        // Polls controller input and converts any changes in state as events
        // Disconnects controller if that case is detected
        void PollState(uint32_t controllerIndex);

        // Return true if the specified controller is connected
        // If it is not connected, the polled input is not valid
        bool IsConnected(uint32_t controllerIndex) const;
        bool IsButtonDown(uint32_t index, uint32_t controllerButtonIndex) const;

        // Set Vibration States
        void SetVibration(uint32_t controllerIndex, float leftPercent, float rightPercent);

    private:
        // Tracks which controllers are connected
        uint32_t m_connectionStatus[XUSER_MAX_COUNT];
        // Is the controller status initialized
        uint32_t m_initialized[XUSER_MAX_COUNT];
        // Previous controller states. Only valid if controller is connected
        XINPUT_STATE m_polledStates[XUSER_MAX_COUNT];
    };
};
