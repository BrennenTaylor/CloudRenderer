#include "WindowFactory.h"
#include "IWindow.h"

#include "Win32/Win32Window.h"

namespace Farlor
{
    IWindow* WindowFactory::CreateWindowInstance()
    {
        return new Win32Window();
    }
}