#include "Win32Window.h"

#include "../WindowEvents.h"
#include "../MouseButtons.h"

#include <windowsx.h>

#include <iostream>

namespace Farlor
{
    Win32Window::Win32Window()
        : m_clientWidth{0}
        , m_clientHeight{0}
        , m_actualWidth{0}
        , m_actualHeight{ 0 }
        , m_isFullscreen{ false }
        , m_isShown{false}
        , m_settingCursor{true}
    {
        // Install basic handler, simply deletes the event
        m_windowEventCallback = [](WindowEvent* pEvent)
        {
            delete pEvent;
        };
    }

    void Win32Window::Initialize(uint32_t desiredClientWidth, uint32_t desiredClientHeight, bool fullscreen, bool startShown)
    {
        m_clientWidth = desiredClientWidth;
        m_clientHeight = desiredClientHeight;
        m_isFullscreen = fullscreen;

        // Set the main window
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        windowClass.lpfnWndProc = WndProc;
        windowClass.hInstance = GetModuleHandle(0);
        windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
        windowClass.hIconSm = windowClass.hIcon;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        windowClass.lpszClassName = "WindowClass";
        windowClass.cbWndExtra = sizeof(Win32Window*);

        RegisterClassEx(&windowClass);

        DEVMODE screenSettings = { 0 };
        int posX, posY;

        if (m_isFullscreen)
        {
            m_clientWidth = GetSystemMetrics(SM_CXSCREEN);
            m_clientWidth = GetSystemMetrics(SM_CYSCREEN);
            screenSettings.dmSize = sizeof(screenSettings);
            screenSettings.dmPelsWidth = static_cast<unsigned long>(m_clientWidth);
            screenSettings.dmPelsHeight = static_cast<unsigned long>(m_clientHeight);
            screenSettings.dmBitsPerPel = 32;
            screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);

            posX = posY = 0;
            m_actualWidth = m_clientWidth;
            m_actualHeight = m_clientHeight;

            m_windowHandle = CreateWindowEx(
                WS_EX_APPWINDOW,
                "WindowClass",
                "Window Title",
                WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP,
                posX, posY,
                m_actualWidth, m_actualHeight,
                0, 0, GetModuleHandle(0), (LPVOID)this);
        }
        else
        {
            RECT wr = { 0, 0, static_cast<long>(m_clientWidth), static_cast<long>(m_clientHeight) };
            AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW, false, 0);

            m_actualWidth = wr.right - wr.left;
            m_actualHeight = wr.bottom - wr.top;

            posX = (GetSystemMetrics(SM_CXSCREEN) - m_actualWidth) / 2;
            posY = (GetSystemMetrics(SM_CYSCREEN) - m_actualHeight) / 2;

            m_windowHandle = CreateWindowEx(
                WS_EX_APPWINDOW,
                "WindowClass",
                "Window Title",
                WS_OVERLAPPEDWINDOW,// | WS_POPUP,
                posX, posY,
                m_actualWidth, m_actualHeight,
                0, 0, GetModuleHandle(0), (LPVOID)this);

            // Set the window handle to be long and point at this window
            SetWindowLongPtr(m_windowHandle, 0, reinterpret_cast<LONG_PTR>(this));
        }
    }

    void Win32Window::Shutdown()
    {
        DestroyWindow(m_windowHandle);
    }

    void Win32Window::ProcessEvent()
    {
        MSG msg = { 0 };

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                WindowEvent* pEvent = new WindowEvent();
                pEvent->type = WindowEventType::WindowClosedEvent;
                pEvent->WindowEventArgs.windowClosedEventArgs;
                m_windowEventCallback(pEvent);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    uint32_t Win32Window::GetClientWidth() const
    {
        return m_clientWidth;
    }

    uint32_t Win32Window::GetClientHeight() const
    {
        return m_clientHeight;
    }

    uint32_t Win32Window::GetActualWidth() const
    {
        return m_actualWidth;
    }

    uint32_t Win32Window::GetActualHeight() const
    {
        return m_actualHeight;
    }

    void Win32Window::ShowGameWindow()
    {
        ShowWindow(m_windowHandle, SW_SHOW);
        SetForegroundWindow(m_windowHandle);
        SetFocus(m_windowHandle);
    }

    const void* Win32Window::GetNativeHandle() const
    {
        return reinterpret_cast<const void*>(&m_windowHandle);
    }

    Vector2 Win32Window::GetWindowCenter()
    {
        RECT windowRect;
        GetWindowRect(m_windowHandle, &windowRect);
        Vector2 center{ static_cast<float>(windowRect.right + windowRect.left) / 2, static_cast<float>(windowRect.top + windowRect.bottom) / 2 };
        return center;
    }

    void Win32Window::SetCursorPos(uint32_t x, uint32_t y)
    {
        m_settingCursor = true;
        ::SetCursorPos(x, y);
    }

    bool Win32Window::IsFullscreen()
    {
        return m_isFullscreen;
    }

    void Win32Window::SetWindowTitle(std::string windowTitle)
    {
        SetWindowText(m_windowHandle, windowTitle.c_str());
    }

    // Assumption is made that the event callback function will clean up memory
    LRESULT CALLBACK Win32Window::RealWndProc(HWND hwnd, UINT message,
        WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        case WM_SIZE:
        {
            WindowEvent* pEvent = new WindowEvent();
            pEvent->type = WindowEventType::WindowResizeEvent;
            pEvent->WindowEventArgs.windowResizeEventArgs.newWidth = (uint32_t)LOWORD(lParam);
            pEvent->WindowEventArgs.windowResizeEventArgs.newHeight = (uint32_t)HIWORD(lParam);
            m_windowEventCallback(pEvent);
        } break;

        case WM_LBUTTONDOWN:
        {
            WindowEvent* pEvent = new WindowEvent();
            pEvent->type = WindowEventType::MouseButtonEvent;
            pEvent->WindowEventArgs.mouseButtonEventArgs.button = Farlor::WindowMouseButtons::Left;
            pEvent->WindowEventArgs.mouseButtonEventArgs.buttonState = WindowMouseButtonState::Down;
            m_windowEventCallback(pEvent);
        } break;

        case WM_LBUTTONUP:
        {
            WindowEvent* pEvent = new WindowEvent();
            pEvent->type = WindowEventType::MouseButtonEvent;
            pEvent->WindowEventArgs.mouseButtonEventArgs.button = Farlor::WindowMouseButtons::Left;
            pEvent->WindowEventArgs.mouseButtonEventArgs.buttonState = WindowMouseButtonState::Up;
            m_windowEventCallback(pEvent);
        } break;

        case WM_RBUTTONDOWN:
        {
            // Able to use same ptr, callback responsible for deleting event memory
            WindowEvent* pEvent = new WindowEvent();
            pEvent->type = WindowEventType::MouseButtonEvent;
            pEvent->WindowEventArgs.mouseButtonEventArgs.button = Farlor::WindowMouseButtons::Right;
            pEvent->WindowEventArgs.mouseButtonEventArgs.buttonState = WindowMouseButtonState::Down;
            m_windowEventCallback(pEvent);
        } break;

        case WM_RBUTTONUP:
        {

            // Able to use same ptr, callback responsible for deleting event memory
            WindowEvent* pEvent = new WindowEvent();
            pEvent->type = WindowEventType::MouseButtonEvent;
            pEvent->WindowEventArgs.mouseButtonEventArgs.button = Farlor::WindowMouseButtons::Right;
            pEvent->WindowEventArgs.mouseButtonEventArgs.buttonState = WindowMouseButtonState::Up;
            m_windowEventCallback(pEvent);
        } break;

        case WM_MOUSEWHEEL:
        {
        } break;

        case WM_MOUSEMOVE:
        {
            if (m_settingCursor)
            {
                m_settingCursor = false;
                break;
            }

            WindowEvent* pEvent = new WindowEvent();
            int32_t newX = GET_X_LPARAM(lParam);
            int32_t newY = GET_Y_LPARAM(lParam);

            POINT p;
            p.x = newX;
            p.y = newY;
            ClientToScreen(m_windowHandle, &p);

            pEvent->type = WindowEventType::MouseMoveEvent;
            pEvent->WindowEventArgs.mouseMoveEventArgs.xPos = p.x;
            pEvent->WindowEventArgs.mouseMoveEventArgs.yPos = p.y;
            m_windowEventCallback(pEvent);
        } break;

        // Keyboard Messages

        // TODO: Need to handle numeric cases as well as other cases that may arise...?
        case WM_KEYDOWN:
        {
            auto DoKeyDown = [&](WindowKeyboardButton button)
            {
                WindowEvent* pEvent = new WindowEvent();
                pEvent->type = WindowEventType::KeyboardButtonEvent;
                pEvent->WindowEventArgs.keyboardButtonEventArgs.button = button;
                pEvent->WindowEventArgs.keyboardButtonEventArgs.buttonState = WindowKeyboardButtonState::Down;
                m_windowEventCallback(pEvent);
            };

            switch (wParam)
            {
            case VK_ESCAPE:
            {
                DoKeyDown(WindowKeyboardButton::Escape);
            } break;


            case VK_BACK:
            {
                DoKeyDown(WindowKeyboardButton::Backspace);
            } break;

            case VK_RETURN:
            {
                DoKeyDown(WindowKeyboardButton::Return);
            } break;

            case VK_SPACE:
            {
                DoKeyDown(WindowKeyboardButton::Space);
            } break;

            case VK_CONTROL:
            {
                DoKeyDown(WindowKeyboardButton::Ctrl);
            } break;

            case VK_SHIFT:
            {
                DoKeyDown(WindowKeyboardButton::Shift);
            } break;

            case VK_LEFT:
            {
                DoKeyDown(WindowKeyboardButton::LeftArrow);
            } break;

            case VK_RIGHT:
            {
                DoKeyDown(WindowKeyboardButton::RightArrow);
            } break;

            case VK_UP:
            {
                DoKeyDown(WindowKeyboardButton::UpArrow);
            } break;

            case VK_DOWN:
            {
                DoKeyDown(WindowKeyboardButton::DownArrow);
            } break;


            case 'A':
            {
                DoKeyDown(WindowKeyboardButton::A);
            } break;

            case 'B':
            {
                DoKeyDown(WindowKeyboardButton::B);
            } break;

            case 'C':
            {
                DoKeyDown(WindowKeyboardButton::C);
            } break;

            case 'D':
            {
                DoKeyDown(WindowKeyboardButton::D);
            } break;

            case 'E':
            {
                DoKeyDown(WindowKeyboardButton::E);
            } break;

            case 'F':
            {
                DoKeyDown(WindowKeyboardButton::F);
            } break;

            case 'G':
            {
                DoKeyDown(WindowKeyboardButton::G);
            } break;

            case 'H':
            {
                DoKeyDown(WindowKeyboardButton::H);
            } break;

            case 'I':
            {
                DoKeyDown(WindowKeyboardButton::I);
            } break;

            case 'J':
            {
                DoKeyDown(WindowKeyboardButton::J);
            } break;

            case 'K':
            {
                DoKeyDown(WindowKeyboardButton::K);
            } break;

            case 'L':
            {
                DoKeyDown(WindowKeyboardButton::L);
            } break;

            case 'M':
            {
                DoKeyDown(WindowKeyboardButton::M);
            } break;

            case 'N':
            {
                DoKeyDown(WindowKeyboardButton::N);
            } break;

            case 'O':
            {
                DoKeyDown(WindowKeyboardButton::O);
            } break;

            case 'P':
            {
                DoKeyDown(WindowKeyboardButton::P);
            } break;

            case 'Q':
            {
                DoKeyDown(WindowKeyboardButton::Q);
            } break;

            case 'R':
            {
                DoKeyDown(WindowKeyboardButton::R);
            } break;

            case 'S':
            {
                DoKeyDown(WindowKeyboardButton::S);
            } break;

            case 'T':
            {
                DoKeyDown(WindowKeyboardButton::T);
            } break;

            case 'U':
            {
                DoKeyDown(WindowKeyboardButton::U);
            } break;

            case 'V':
            {
                DoKeyDown(WindowKeyboardButton::V);
            } break;

            case 'W':
            {
                DoKeyDown(WindowKeyboardButton::W);
            } break;

            case 'X':
            {
                DoKeyDown(WindowKeyboardButton::X);
            } break;

            case 'Y':
            {
                DoKeyDown(WindowKeyboardButton::Y);
            } break;

            case 'Z':
            {
                DoKeyDown(WindowKeyboardButton::Z);
            } break;

            case VK_OEM_3:
            {
                DoKeyDown(WindowKeyboardButton::Tilda);
            } break;

            default:
            {
            } break;
            };
        } break;

        case WM_KEYUP:
        {
            auto DoKeyUp = [&](WindowKeyboardButton button)
            {
                WindowEvent* pEvent = new WindowEvent();
                pEvent->type = WindowEventType::KeyboardButtonEvent;
                pEvent->WindowEventArgs.keyboardButtonEventArgs.button = button;
                pEvent->WindowEventArgs.keyboardButtonEventArgs.buttonState = WindowKeyboardButtonState::Up;
                m_windowEventCallback(pEvent);
            };

            switch (wParam)
            {
            case VK_ESCAPE:
            {
                DoKeyUp(WindowKeyboardButton::Escape);
            } break;


            case VK_BACK:
            {
                DoKeyUp(WindowKeyboardButton::Backspace);
            } break;

            case VK_RETURN:
            {
                DoKeyUp(WindowKeyboardButton::Return);
            } break;

            case VK_SPACE:
            {
                DoKeyUp(WindowKeyboardButton::Space);
            } break;

            case VK_CONTROL:
            {
                DoKeyUp(WindowKeyboardButton::Ctrl);
            } break;

            case VK_SHIFT:
            {
                DoKeyUp(WindowKeyboardButton::Shift);
            } break;


            case VK_LEFT:
            {
                DoKeyUp(WindowKeyboardButton::LeftArrow);
            } break;

            case VK_RIGHT:
            {
                DoKeyUp(WindowKeyboardButton::RightArrow);
            } break;

            case VK_UP:
            {
                DoKeyUp(WindowKeyboardButton::UpArrow);
            } break;

            case VK_DOWN:
            {
                DoKeyUp(WindowKeyboardButton::DownArrow);
            } break;

            case 'A':
            {
                DoKeyUp(WindowKeyboardButton::A);
            } break;

            case 'B':
            {
                DoKeyUp(WindowKeyboardButton::B);
            } break;

            case 'C':
            {
                DoKeyUp(WindowKeyboardButton::C);
            } break;

            case 'D':
            {
                DoKeyUp(WindowKeyboardButton::D);
            } break;

            case 'E':
            {
                DoKeyUp(WindowKeyboardButton::E);
            } break;

            case 'F':
            {
                DoKeyUp(WindowKeyboardButton::F);
            } break;

            case 'G':
            {
                DoKeyUp(WindowKeyboardButton::G);
            } break;

            case 'H':
            {
                DoKeyUp(WindowKeyboardButton::H);
            } break;

            case 'I':
            {
                DoKeyUp(WindowKeyboardButton::I);
            } break;

            case 'J':
            {
                DoKeyUp(WindowKeyboardButton::J);
            } break;

            case 'K':
            {
                DoKeyUp(WindowKeyboardButton::K);
            } break;

            case 'L':
            {
                DoKeyUp(WindowKeyboardButton::L);
            } break;

            case 'M':
            {
                DoKeyUp(WindowKeyboardButton::M);
            } break;

            case 'N':
            {
                DoKeyUp(WindowKeyboardButton::N);
            } break;

            case 'O':
            {
                DoKeyUp(WindowKeyboardButton::O);
            } break;

            case 'P':
            {
                DoKeyUp(WindowKeyboardButton::P);
            } break;

            case 'Q':
            {
                DoKeyUp(WindowKeyboardButton::Q);
            } break;

            case 'R':
            {
                DoKeyUp(WindowKeyboardButton::R);
            } break;

            case 'S':
            {
                DoKeyUp(WindowKeyboardButton::S);
            } break;

            case 'T':
            {
                DoKeyUp(WindowKeyboardButton::T);
            } break;

            case 'U':
            {
                DoKeyUp(WindowKeyboardButton::U);
            } break;

            case 'V':
            {
                DoKeyUp(WindowKeyboardButton::V);
            } break;

            case 'W':
            {
                DoKeyUp(WindowKeyboardButton::W);
            } break;

            case 'X':
            {
                DoKeyUp(WindowKeyboardButton::X);
            } break;

            case 'Y':
            {
                DoKeyUp(WindowKeyboardButton::Y);
            } break;

            case 'Z':
            {
                DoKeyUp(WindowKeyboardButton::Z);
            } break;

            case VK_OEM_3:
            {
                DoKeyUp(WindowKeyboardButton::Tilda);
            } break;

            default:
            {
            } break;
            };
        } break;

        default:
        {
            return DefWindowProc(hwnd, message, wParam, lParam);
        } break;
        }

        return 0;
    }

    LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT message,
        WPARAM wParam, LPARAM lParam)
    {
        Win32Window* pWindow = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, 0));
        if (pWindow)
        {
            return pWindow->RealWndProc(hwnd, message, wParam, lParam);
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}