#include "StringUtil.h"

#include <codecvt>
#include <iostream>
#include <locale>
#include <Windows.h>

namespace Farlor
{
namespace Utility
{
    std::wstring StringUtil::StringToWideString(const std::string& str)
    {
        if (str.empty())
        {
            return std::wstring();
        }
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    std::string StringUtil::WideStringToString(const std::wstring& wstr)
    {
        if (wstr.empty())
        {
            return std::string();
        }
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    void StringUtil::LTrim(std::string& str)
    {
        auto first = str.find_first_not_of(' ');
        if (std::string::npos == first)
        {
            return;
        }
        str = str.substr(first, std::string::npos);
    }
    
    void StringUtil::RTrim(std::string& str)
    {
        auto end = str.find_last_not_of(' ');
        if (std::string::npos == end)
        {
            return;
        }

        size_t start = 0;
        str = str.substr(start, end+1);
    }

    void StringUtil::Trim(std::string& str)
    {
        LTrim(str);
        RTrim(str);
    }

    std::string StringUtil::GetFirstWord(const std::string& str)
    {
        size_t first = 0;
        std::string firstWord = str.substr(first, str.find(' '));
        return firstWord;
    }

    std::string StringUtil::GetAndRemoveFirstWord(std::string& str)
    {
        size_t first = 0;
        size_t whitespace = str.find(' ');
        std::string firstWord = str.substr(first, whitespace);

        if (whitespace == std::string::npos)
        {
            return firstWord;
        }

        str = str.substr(whitespace, std::string::npos);
        LTrim(str);
        return firstWord;
    }

    std::string StringUtil::RemoveFilenameFromPath(const std::string filePath)
    {
        return filePath.substr(0, filePath.find_last_of("\\/") + 1);
    }
}
}