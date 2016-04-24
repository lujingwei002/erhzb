#ifndef _CJSON_H_
#define _CJSON_H_

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

/*
 *
 * 
 * Json.encode({a='a', b='b'})
 * Json.decode(str)
 *
 *
 *
 * //str to lua table
 * int json_decode(lua_State *L, char *str)
 *
 * //lua table to str
 * char *json_encode(lua_State *L)
 *
 *
 *
 */

char *json_encode(lua_State *L);
int json_decode(lua_State* L, char *str);

int luaopen_json(lua_State *L);

#endif
