#pragma once

#include <cstdint>

namespace Farlor
{
    const uint32_t MaxNumControllers = 4;
    namespace ControllerButtons
    {
        const uint32_t X = 0;
        const uint32_t Y = X + 1;
        const uint32_t B = Y + 1;
        const uint32_t A = B + 1;
        const uint32_t Select = A + 1;
        const uint32_t Start = Select + 1;
        const uint32_t DPadLeft = Start + 1;
        const uint32_t DPadUp = DPadLeft + 1;
        const uint32_t DPadRight = DPadUp + 1;
        const uint32_t DPadDown = DPadRight + 1;
        const uint32_t LeftBumper = DPadDown + 1;
        const uint32_t RightBumper = LeftBumper + 1;
        const uint32_t LeftThumb = RightBumper + 1;
        const uint32_t RightThumb = LeftThumb + 1;
        const uint32_t NumControllerButtons = RightThumb + 1;
    };

    namespace Controllers
    {
        const uint32_t Controller0 = 0;
        const uint32_t Controller1 = 1;
        const uint32_t Controller2 = 2;
        const uint32_t Controller3 = 3;
    };
}