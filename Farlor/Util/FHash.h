#pragma once

#include <cstdint>

namespace Farlor
{
namespace Utility
{
    class FHash
    {
    public:
        static uint32_t Hash(const uint8_t* pData, size_t size);
    };
}
}