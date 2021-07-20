#include "../Farlor.h"

#include <memory>

#pragma warning(push)
#pragma warning(disable : 4067)
#include <snappy.h>
#pragma warning(pop)

namespace Farlor
{
namespace Utility
{
    class FCompress
    {
    public:
        struct Span
        {
            std::shared_ptr<u8> spData;
            size_t size;
        };

        static Span Compress(const u8* pData, size_t size);
        static Span Uncompress(const u8* pData, size_t size);
    };
}
}