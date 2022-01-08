#pragma once

#include <cstdint>

namespace Farlor
{
    enum class ProfileEvent : uint32_t
    {
        ClearBuffers = 0,
        CloudTrace = ClearBuffers + 1,
        Tonemap = CloudTrace + 1,
        GBuffer = Tonemap + 1,
        NumEvents = GBuffer + 1
    };
}