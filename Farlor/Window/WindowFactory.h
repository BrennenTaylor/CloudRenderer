#pragma once

#include <Windows.h>

namespace Farlor
{
    class IWindow;

    class WindowFactory
    {
    public:
        IWindow* CreateWindowInstance();
    };
}