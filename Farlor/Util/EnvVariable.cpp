#include "EnvVariable.h"

using namespace std;

namespace Farlor
{
    string EnvVariable::GetEnvVariable(string name)
    {
#ifdef _WIN32
        char* envValue = nullptr;
        size_t size = 0;
        if(_dupenv_s(&envValue, &size, name.c_str()) != 0)
        {
            free(envValue);
            envValue = nullptr;
        }
#else
        char* envValue = getenv(name.c_str());
#endif

        string result;
        if (envValue)
        {
            result = string(envValue);
        }

        #ifdef _WIN32
                free(envValue);
        #endif

        return result;
    }
}