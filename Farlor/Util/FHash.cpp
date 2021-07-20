#include "FHash.h"

namespace Farlor
{
namespace Utility
{
    // Uses the x65599 hash function.
    // Red dragon book
    uint32_t FHash::Hash(const uint8_t* pData, size_t size)
    {
        uint32_t hash = 0;
        for (size_t i = 0; i < size; ++i)
        {
            hash = 65599 * hash + pData[i];
        }

        return hash ^ (hash >> 16);
    }
}
}