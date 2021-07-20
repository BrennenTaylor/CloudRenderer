#pragma once

#include <string>

namespace Farlor
{
namespace Utility
{
    class FEnvVar
    {
    public:
        static std::string GetEnvVar(std::string name);
    };
}
}