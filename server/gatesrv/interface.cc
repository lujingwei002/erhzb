/*
** Lua binding: gatesrv
** Generated automatically by tolua++-1.0.92 on Tue Apr 26 11:07:43 2016.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_gatesrv_open (lua_State* tolua_S);

#include "websocket/websocket.h"
#include "gameclient/gameclient.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: Websocket::init */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Websocket_init00
static int tolua_gatesrv_Websocket_init00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Websocket::init();
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

/* function: Websocket::listen */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Websocket_listen00
static int tolua_gatesrv_Websocket_listen00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Websocket::listen(host,port);
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

/* function: Websocket::send_string_frame */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Websocket_send_string_frame00
static int tolua_gatesrv_Websocket_send_string_frame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int sid = ((int)  tolua_tonumber(tolua_S,1,0));
  const char* str = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   int tolua_ret = (int)  Websocket::send_string_frame(sid,str);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_string_frame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Websocket::send_binary_frame */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Websocket_send_binary_frame00
static int tolua_gatesrv_Websocket_send_binary_frame00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Websocket::send_binary_frame(sid,data,datalen);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_binary_frame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gameclient::init */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_init00
static int tolua_gatesrv_Gameclient_init00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gameclient::init();
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

/* function: Gameclient::connect */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_connect00
static int tolua_gatesrv_Gameclient_connect00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gameclient::connect(host,port);
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

/* function: Gameclient::is_connect */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_is_connect00
static int tolua_gatesrv_Gameclient_is_connect00(lua_State* tolua_S)
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
   bool tolua_ret = (bool)  Gameclient::is_connect();
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

/* function: Gameclient::send_data */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_send_data00
static int tolua_gatesrv_Gameclient_send_data00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gameclient::send_data(sid,data,datalen);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_data'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gameclient::send_session_open */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_send_session_open00
static int tolua_gatesrv_Gameclient_send_session_open00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int sid = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   int tolua_ret = (int)  Gameclient::send_session_open(sid);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_session_open'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gameclient::send_session_close */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_send_session_close00
static int tolua_gatesrv_Gameclient_send_session_close00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int sid = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   int tolua_ret = (int)  Gameclient::send_session_close(sid);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_session_close'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Gameclient::update */
#ifndef TOLUA_DISABLE_tolua_gatesrv_Gameclient_update00
static int tolua_gatesrv_Gameclient_update00(lua_State* tolua_S)
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
   int tolua_ret = (int)  Gameclient::update();
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

/* Open function */
TOLUA_API int tolua_gatesrv_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"Websocket",0);
  tolua_beginmodule(tolua_S,"Websocket");
   tolua_function(tolua_S,"init",tolua_gatesrv_Websocket_init00);
   tolua_function(tolua_S,"listen",tolua_gatesrv_Websocket_listen00);
   tolua_function(tolua_S,"send_string_frame",tolua_gatesrv_Websocket_send_string_frame00);
   tolua_function(tolua_S,"send_binary_frame",tolua_gatesrv_Websocket_send_binary_frame00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"Gameclient",0);
  tolua_beginmodule(tolua_S,"Gameclient");
   tolua_function(tolua_S,"init",tolua_gatesrv_Gameclient_init00);
   tolua_function(tolua_S,"connect",tolua_gatesrv_Gameclient_connect00);
   tolua_function(tolua_S,"is_connect",tolua_gatesrv_Gameclient_is_connect00);
   tolua_function(tolua_S,"send_data",tolua_gatesrv_Gameclient_send_data00);
   tolua_function(tolua_S,"send_session_open",tolua_gatesrv_Gameclient_send_session_open00);
   tolua_function(tolua_S,"send_session_close",tolua_gatesrv_Gameclient_send_session_close00);
   tolua_function(tolua_S,"update",tolua_gatesrv_Gameclient_update00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 extern "C"{
 TOLUA_API int luaopen_gatesrv (lua_State* tolua_S) {
 return tolua_gatesrv_open(tolua_S);
};
}
#endif

