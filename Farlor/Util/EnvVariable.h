#pragma once

#include <string>

namespace Farlor
{
    class EnvVariable
    {
    public:
        static std::string GetEnvVariable(std::string name);
    };
}