#include "stdafx.h"

 extern "C"{
 int luaopen_dbsrv(lua_State* tolua_S);
}

namespace Script
{
    lua_State* L;
    int init()
    {
        L = lua_open();
        luaL_openlibs(L);
        luaopen_dbsrv(L);
        luaopen_mysql(L);
        
        if(dofile("main.lua"))
        {
            return 1;
        }

        return 0;
    }

    int dofile(const char* filepath)
    {
        if(luaL_dofile(L, filepath))
        {
            if (lua_isstring(L, -1))
            {
                LOG("dofile error %s\n", lua_tostring(L, -1));
                return 1;
            }
        }
        return 0;
    }

    int pushluafunction(const char *func)
    {
        char *start = (char *)func;
        char *class_name = start;
        char *pfunc = start;
        while(*pfunc != 0)
        {
            if(*pfunc == '.' && class_name == start)
            {
                *pfunc = 0;
                lua_getglobal(L, class_name);
                *pfunc = '.';
                if(lua_isnil(L, -1)){
                    printf("eeeeeeeeeeeeeeeeeeeee\n");
                    return 1;
                }
                class_name = pfunc + 1;
            }else if(*pfunc == '.')
            {
                *pfunc = 0;
                lua_pushstring(L, class_name);
                lua_gettable(L, -2);
                *pfunc = '.';
                if(lua_isnil(L, -1))
                {
                    return 2;
                }
                lua_remove(L, -2);//弹出table
                class_name = pfunc + 1;
            }
            pfunc++;
        }
        if(class_name == start)
        {
            lua_getglobal(L, class_name);
            if(lua_isnil(L, -1))
            {
                return 3;
            }
        }else
        {
            lua_pushstring(L, class_name);
            lua_gettable(L, -2);
            if(lua_isnil(L, -1))
            {
                return 4;
            }
            lua_remove(L, -2);//弹出table
        }
        return 0;     
    }

    int printluastack() 
    {
        lua_getglobal(L, "debug");  
        lua_getfield(L, -1, "traceback");  
        lua_pcall(L, 0, 1, 0);   
        const char* sz = lua_tostring(L, -1);  
        printf("%s\n", sz);
        return 0;
    }

};
