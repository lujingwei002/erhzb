/*
** Lua binding: gamesrv
** Generated automatically by tolua++-1.0.92 on Fri Apr  8 11:14:37 2016.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_gamesrv_open (lua_State* tolua_S);

#include "testfile/item.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"User");
}

/* method: hello of class  User */
#ifndef TOLUA_DISABLE_tolua_gamesrv_User_hello00
static int tolua_gamesrv_User_hello00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"User",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  User* self = (User*)  tolua_tousertype(tolua_S,1,0);
  char* str = ((char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'hello'",NULL);
#endif
  {
   int tolua_ret = (int)  self->hello(str);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'hello'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: uid of class  User */
#ifndef TOLUA_DISABLE_tolua_get_User_uid
static int tolua_get_User_uid(lua_State* tolua_S)
{
  User* self = (User*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'uid'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->uid);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: uid of class  User */
#ifndef TOLUA_DISABLE_tolua_set_User_uid
static int tolua_set_User_uid(lua_State* tolua_S)
{
  User* self = (User*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'uid'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->uid = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: Item::add_item */
#ifndef TOLUA_DISABLE_tolua_gamesrv_Item_add_item00
static int tolua_gamesrv_Item_add_item00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int itemid = ((int)  tolua_tonumber(tolua_S,1,0));
  int itemcount = ((int)  tolua_tonumber(tolua_S,2,0));
  {
   int tolua_ret = (int)  Item::add_item(itemid,itemcount);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'add_item'.",&tolua_err);
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
  tolua_cclass(tolua_S,"User","User","",NULL);
  tolua_beginmodule(tolua_S,"User");
   tolua_function(tolua_S,"hello",tolua_gamesrv_User_hello00);
   tolua_variable(tolua_S,"uid",tolua_get_User_uid,tolua_set_User_uid);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"Item",0);
  tolua_beginmodule(tolua_S,"Item");
   tolua_function(tolua_S,"add_item",tolua_gamesrv_Item_add_item00);
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

