#pragma once

#include <string>

namespace Farlor
{
namespace Utility
{
    class StringUtil
    {
    public:
        static std::wstring StringToWideString(const std::string& str);
        static std::string WideStringToString(const std::wstring& wstr);

        static void Trim(std::string& str);
        static void LTrim(std::string& str);
        static void RTrim(std::string& str);

        static std::string GetFirstWord(const std::string& str);
        static std::string GetAndRemoveFirstWord(std::string& str);

        // Filepath operations
        static std::string RemoveFilenameFromPath(const std::string filePath);
    };
}
}
