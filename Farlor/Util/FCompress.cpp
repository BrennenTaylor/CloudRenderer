#include "FCompress.h"

namespace Farlor
{
namespace Utility
{
    FCompress::Span FCompress::Compress(const u8* pData, size_t size)
    {
        Span compressed{ nullptr, 0 };

        // If no vaid data to compress
        if (!pData || !size)
        {
            return compressed;
        }

        size_t maxLength = snappy::MaxCompressedLength(size);
        std::shared_ptr<u8> spCompressed(new u8[maxLength], std::default_delete<u8[]>());
        size_t compressedLength;
        snappy::RawCompress(reinterpret_cast<const char*>(pData), size, reinterpret_cast<char*>(spCompressed.get()), &compressedLength);

        compressed.spData = spCompressed;
        compressed.size = compressedLength;
        return compressed;
    }

    FCompress::Span FCompress::Uncompress(const u8* pData, size_t size)
    {
        Span uncompressed{ nullptr, 0 };

        // If no vaid data to uncompress
        if (!pData || !size)
        {
            return uncompressed;
        }

        size_t uncompressedLength;
        snappy::GetUncompressedLength(reinterpret_cast<const char*>(pData), size, &uncompressedLength);
        std::shared_ptr<u8> spUncompressed(new u8[uncompressedLength], std::default_delete<u8[]>());
        snappy::RawUncompress(reinterpret_cast<const char*>(pData), size, reinterpret_cast<char*>(spUncompressed.get()));

        uncompressed.spData = spUncompressed;
        uncompressed.size = uncompressedLength;
        return uncompressed;
    }
}
}