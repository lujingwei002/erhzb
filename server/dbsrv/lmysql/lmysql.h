#ifndef _L_MYSQL_H_
#define _L_MYSQL_H_

#include <mysql.h>
#include <stdlib.h>
extern "C"{ 
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}



int luaopen_mysql(lua_State *L);
#endif
