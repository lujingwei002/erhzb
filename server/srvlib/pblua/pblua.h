#ifndef _PBC_H_
#define _PBC_H_

#include <string.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <iostream>

extern "C"{
#include <lua.h> 
#include <lualib.h>
#include <lauxlib.h>
}

void pbc_stat();

typedef struct LuaMessage {
    google::protobuf::Message *message;
    LuaMessage *root_message;
    int dirty;
}LuaMessage;

typedef struct RepeatedField{
    LuaMessage *message;
    const google::protobuf::FieldDescriptor *field;
}RepeatedField;

int pbc_import(const char *file_name);
google::protobuf::Message* pbc_load_msg(const char* type_name);
int luaopen_pblua(lua_State *L);

#endif
