extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace Script
{

    extern lua_State* L;
    int dofile(const char* filepath);
    int pushluafunction(const char *func);
    int printluastack(); 
    int init();
};
