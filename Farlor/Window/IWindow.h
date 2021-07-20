#pragma once

#include "WindowEvents.h"

#include <FMath/FMath.h>

#include <cstdint>
#include <functional>

namespace Farlor
{
    class IWindow;

    // This is the interface implemented by each of the window libraries
    class IWindow
    {
    public:
        virtual void Initialize(uint32_t desiredClientWidth, uint32_t desiredClientHeight, bool fullscreen, bool startShown) = 0;
        virtual void Shutdown() = 0;

        virtual void ProcessEvent() = 0;

        virtual uint32_t GetClientWidth() const = 0;
        virtual uint32_t GetClientHeight() const = 0;
        virtual uint32_t GetActualWidth() const = 0;
        virtual uint32_t GetActualHeight() const = 0;

        virtual void ShowGameWindow() = 0;

        virtual Vector2 GetWindowCenter() = 0;
        virtual void SetCursorPos(uint32_t x, uint32_t y) = 0;

        virtual bool IsFullscreen() = 0;

        virtual const void* GetNativeHandle() const = 0;

        virtual void SetWindowTitle(std::string windowTitle) = 0;

        void SetWindowEventCallback(const std::function<void(WindowEvent* WindowEvent)>& windowEventCallback)
        {
            m_windowEventCallback = windowEventCallback;
        }

    protected:
        std::function<void(WindowEvent* WindowEvent)> m_windowEventCallback;
    };
}