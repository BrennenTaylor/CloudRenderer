#pragma once

#include <string>

namespace Farlor
{
    class HashedString
    {
    public:
        HashedString();
        HashedString(const std::string& stringToHash);

        static uint32_t HashString(const std::string& stringToHash);

        bool operator<(const HashedString& other) const
        {
            return (m_id < other.m_id);
        }
        bool operator==(const HashedString& other) const
        {
            return (m_id == other.m_id);
        }

        // Store as void* so it is displayed as hex for debugger
        uint32_t m_id;
        std::string m_hashedString;
    };
}
