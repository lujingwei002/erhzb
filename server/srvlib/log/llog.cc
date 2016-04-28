#include "log/log.h"

namespace Log
{
    void log(const char* str)
    {
        Log::log("%s", str);
    }
};
