#pragma once

#include "../IWindow.h"

#include <Windows.h>

#include <cstdint>

namespace Farlor
{
    class Win32Window : public IWindow
    {
    public:
        Win32Window();

        virtual void Initialize(uint32_t desiredClientWidth, uint32_t desiredClientHeight, bool fullscreen, bool startShown) override;
        virtual void Shutdown() override;

        virtual void ProcessEvent() override;

        virtual uint32_t GetClientWidth() const override;
        virtual uint32_t GetClientHeight() const override;
        virtual uint32_t GetActualWidth() const override;
        virtual uint32_t GetActualHeight() const override;

        virtual void ShowGameWindow() override;

        virtual Vector2 GetWindowCenter() override;
        virtual void SetCursorPos(uint32_t x, uint32_t y) override;

        virtual bool IsFullscreen() override;

        virtual const void* GetNativeHandle() const override;
        virtual void SetWindowTitle(std::string windowTitle) override;

        // This is able to call into the window specific wndproc due to win32 trickery
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
            WPARAM wParam, LPARAM lParam);

    private:
        LRESULT CALLBACK RealWndProc(HWND hwnd, UINT message,
            WPARAM wParam, LPARAM lParam);

    private:
        uint32_t m_clientWidth;
        uint32_t m_clientHeight;
        uint32_t m_actualWidth;
        uint32_t m_actualHeight;
        uint32_t m_isFullscreen;
        uint32_t m_isShown;

        HWND m_windowHandle;

        bool m_settingCursor;
    };
}