/*
** Lua binding: gamesrv
** Generated automatically by tolua++-1.0.92 on Wed Apr 27 11:26:01 2016.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_gamesrv_open (lua_State* tolua_S);

#include "gatesrv/gatesrv.h"
#include "dbclient/dbclient.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: Gatesrv::init */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Gatesrv_init00
static int tolua_gamesrv_Gatesrv_init00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gatesrv::init();
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

/* function: Gatesrv::listen */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Gatesrv_listen00
static int tolua_gamesrv_Gatesrv_listen00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gatesrv::listen(host,port);
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

/* function: Gatesrv::send */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Gatesrv_send00
static int tolua_gamesrv_Gatesrv_send00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isuserdata(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int sid = ((int)  tolua_tonumber(tolua_S,1,0));
  const void* data = ((const void*)  tolua_touserdata(tolua_S,2,0));
  unsigned short datalen = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  {
   int tolua_ret = (int)  Gatesrv::send(sid,data,datalen);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gatesrv::send_proto */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Gatesrv_send_proto00
static int tolua_gamesrv_Gatesrv_send_proto00(lua_State* tolua_S)
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
return Gatesrv::send_proto(L);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_proto'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Dbclient::init */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_init00
static int tolua_gamesrv_Dbclient_init00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Dbclient::init();
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

/* function: Dbclient::connect */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_connect00
static int tolua_gamesrv_Dbclient_connect00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Dbclient::connect(host,port);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'connect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Dbclient::send */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_send00
static int tolua_gamesrv_Dbclient_send00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int sockfd = ((int)  tolua_tonumber(tolua_S,1,0));
  const char* data = ((const char*)  tolua_tostring(tolua_S,2,0));
  unsigned short datalen = ((unsigned short)  tolua_tonumber(tolua_S,3,0));
  {
   int tolua_ret = (int)  Dbclient::send(sockfd,data,datalen);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Dbclient::post */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_post00
static int tolua_gamesrv_Dbclient_post00(lua_State* tolua_S)
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
return Dbclient::post(L);
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

/* function: Dbclient::update */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_update00
static int tolua_gamesrv_Dbclient_update00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Dbclient::update();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Dbclient::is_connect */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Dbclient_is_connect00
static int tolua_gamesrv_Dbclient_is_connect00(lua_State* tolua_S)
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
   bool tolua_ret = (bool)  Dbclient::is_connect();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'is_connect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_gamesrv_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"Gatesrv",0);
  tolua_beginmodule(tolua_S,"Gatesrv");
   tolua_function(tolua_S,"init",tolua_gamesrv_Gatesrv_init00);
   tolua_function(tolua_S,"listen",tolua_gamesrv_Gatesrv_listen00);
   tolua_function(tolua_S,"send",tolua_gamesrv_Gatesrv_send00);
   tolua_function(tolua_S,"send_proto",tolua_gamesrv_Gatesrv_send_proto00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"Dbclient",0);
  tolua_beginmodule(tolua_S,"Dbclient");
   tolua_function(tolua_S,"init",tolua_gamesrv_Dbclient_init00);
   tolua_function(tolua_S,"connect",tolua_gamesrv_Dbclient_connect00);
   tolua_function(tolua_S,"send",tolua_gamesrv_Dbclient_send00);
   tolua_function(tolua_S,"post",tolua_gamesrv_Dbclient_post00);
   tolua_function(tolua_S,"update",tolua_gamesrv_Dbclient_update00);
   tolua_function(tolua_S,"is_connect",tolua_gamesrv_Dbclient_is_connect00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 extern "C"{
 TOLUA_API int luaopen_gamesrv (lua_State* tolua_S) {
 return tolua_gamesrv_open(tolua_S);
};
}
#endif

