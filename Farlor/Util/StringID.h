#pragma once

#include "../Farlor.h"

namespace Farlor
{
    inline u32 djb2(const char* value)
    {
        unsigned char* str = (unsigned char*)value;
        u32 hash = 5381;
        int c;

        while (c = *str++)
            hash = ((hash << 5) + hash) + c;

        return hash;
    }

    inline u32 CheckHash(u32 id, const char *name)
    {
        assert(id == djb2(name));
        return id;
    }

#ifdef _DEBUG
    #define HASHSTRING(id, name) CheckHash(id, name)
#else
    #define HASHSTRING(id, name) id
#endif

    using StrID = u32;
}


