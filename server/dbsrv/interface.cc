/*
** Lua binding: dbsrv
** Generated automatically by tolua++-1.0.92 on Sun Apr 24 18:39:41 2016.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_dbsrv_open (lua_State* tolua_S);

#include "gamesrv/gamesrv.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: Gamesrv::init */
#ifndef TOLUA_DISABLE_tolua_dbsrv_Gamesrv_init00
static int tolua_dbsrv_Gamesrv_init00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   int tolua_ret = (int)  Gamesrv::init();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'init'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gamesrv::listen */
#ifndef TOLUA_DISABLE_tolua_dbsrv_Gamesrv_listen00
static int tolua_dbsrv_Gamesrv_listen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* host = ((const char*)  tolua_tostring(tolua_S,1,0));
  unsigned short port = ((unsigned short)  tolua_tonumber(tolua_S,2,0));
  {
   int tolua_ret = (int)  Gamesrv::listen(host,port);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'listen'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gamesrv::post */
#ifndef TOLUA_DISABLE_tolua_dbsrv_Gamesrv_post00
static int tolua_dbsrv_Gamesrv_post00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     false
 )
  goto tolua_lerror;
 else
#endif
 {
  lua_State* L =  tolua_S;
  {
return Gamesrv::post(L);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'post'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_dbsrv_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"Gamesrv",0);
  tolua_beginmodule(tolua_S,"Gamesrv");
   tolua_function(tolua_S,"init",tolua_dbsrv_Gamesrv_init00);
   tolua_function(tolua_S,"listen",tolua_dbsrv_Gamesrv_listen00);
   tolua_function(tolua_S,"post",tolua_dbsrv_Gamesrv_post00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 extern "C"{
 TOLUA_API int luaopen_dbsrv (lua_State* tolua_S) {
 return tolua_dbsrv_open(tolua_S);
};
}
#endif

