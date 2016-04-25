#include "stdafx.h"
#include <iostream>

class MyMultiFileErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector{
        virtual void AddError(
                const std::string & filename,
                int line,
                int column,
                const std::string & message){
            std::cout << line << ":" << column << " " << message << std::endl;
       }
};

static google::protobuf::compiler::DiskSourceTree *sourceTree;
static MyMultiFileErrorCollector *errorCollector;
static google::protobuf::compiler::Importer *importer;
static const google::protobuf::DescriptorPool *pool;
static google::protobuf::DynamicMessageFactory *factory;

int s_msg_count = 0;
int s_msg_new_times = 0;
int s_msg_delete_times = 0;

void pblua_stat(){
    LOG_LOG("pblua msg count:%d", s_msg_count);
    LOG_LOG("pblua msg new times:%d", s_msg_new_times);
    LOG_LOG("pblua msg delete times:%d", s_msg_delete_times);
}

google::protobuf::MessageFactory* pblua_factory(){
    return factory;
}

google::protobuf::Message* pblua_load_msg(const char* type_name){
    google::protobuf::Message* message = NULL;
    const google::protobuf::Descriptor* descriptor = pool->FindMessageTypeByName(type_name);
    if(descriptor == NULL){
        LOG_LOG("can not find msg %s", type_name);
        return NULL;
    }
    const google::protobuf::Message* prototype = factory->GetPrototype(descriptor);
    if(prototype == NULL){
        LOG_LOG("can not find msg %s", type_name);
        return NULL;
    }
    message = prototype->New();
    if(message == NULL){
        LOG_LOG("can not find msg %s", type_name);
        return NULL;
    }
    s_msg_count++;
    s_msg_new_times++;
    return message;
}

//pbenum.xxx.xxx.xx
static int import_enum(lua_State *L, const google::protobuf::EnumDescriptor* enum_type){
    int top = lua_gettop(L);
    lua_getglobal(L, "pbenum");
    for(int i = 0; i < enum_type->value_count(); i++){
        const google::protobuf::EnumValueDescriptor *value = enum_type->value(i);
        if(value == NULL){
            continue;
        }
        char *full_name = (char *)value->full_name().data();
        char *pstart = full_name;
        char *pfunc = full_name;
        while(*pfunc != 0){
            if(*pfunc == '.'){
                *pfunc = 0;
                lua_pushstring(L, pstart);
                lua_gettable(L, -2);
                if(lua_isnil(L, -1)){
                    lua_pop(L, 1);
                    lua_pushstring(L, pstart);
                    lua_newtable(L);
                    lua_settable(L, -3);
                    lua_pushstring(L, pstart);
                    lua_gettable(L, -2);
                }
                *pfunc = '.';
                pstart = pfunc + 1;
            }
            pfunc++;
        }
        lua_pushstring(L, pstart);
        lua_pushnumber(L, value->number());
        lua_settable(L, -3);
        lua_pop(L, lua_gettop(L) - top - 1);//留下pbenum
    }
    lua_pop(L, lua_gettop(L) - top);
    return 1;    
}

static int ltest(lua_State* L){
    return 0;
}
/*
 *导入proto文件
 * 
 */
static int limport(lua_State* L){
    if(lua_isstring(L, 1)){
        const char *file_name = lua_tostring(L, 1);
        const google::protobuf::FileDescriptor *file = importer->Import(file_name);
        if(file == NULL){
            LOG_LOG("import fail %s", file_name);
            return 0;
        }
        lua_pushboolean(L, 1);
        return 1;
        for(int i = 0; i < file->enum_type_count(); i++){
            const google::protobuf::EnumDescriptor *enum_type = file->enum_type(i);
            if(enum_type == NULL){
                LOG_LOG("null");
                continue;
            }
            import_enum(L, enum_type);
        }
        for(int i = 0; i < file->message_type_count() ;i++){
            const google::protobuf::Descriptor *descriptor = file->message_type(i);
            if(descriptor == NULL){
                LOG_LOG("null");
                continue;
            }
            for(int j = 0; j < descriptor->enum_type_count(); j++){
                const google::protobuf::EnumDescriptor *enum_type = descriptor->enum_type(j);
                if(enum_type == NULL){
                    LOG_LOG("null");
                    continue;
                }
                import_enum(L, enum_type);
            }
        }
        return 1;
    }
    return 0;
}

/*
 * 设置proto文件的导入目录 
 */
static int lmappath(lua_State* L)
{
    if(lua_isstring(L, 1) && lua_isstring(L, 2)){
        const char *name = lua_tostring(L, 1);
        const char *dir = lua_tostring(L, 2);
        sourceTree->MapPath(name, dir);
        sourceTree->MapPath(name, dir);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}
/*
 * 
 * @arg1 msg
 * @arg2 buf
 * @arg3 buflen
 */
static int lmsg_parse_from_buf(lua_State *L){
    LuaMessage *message_lua;
    char *buf;
    size_t str_len;
    message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    buf = (char *)lua_touserdata(L, 2);
    if(buf == NULL){
        LOG_LOG("null");
        return 0;
    }
    str_len = (int)lua_tointeger(L, 3);
    google::protobuf::io::ArrayInputStream stream(buf, str_len);
    if(message->ParseFromZeroCopyStream(&stream) == 0){
        LOG_LOG("parse fail\n");
        lua_pushboolean(L, false);
        return 1;
    }
    lua_pushboolean(L, true);
    return 1;
}


static int lmsg_parse_from_string(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(lua_isstring(L, 2) == 0){
        LOG_LOG("null");
        return 0;
    }
    size_t str_len;
    const char *str = lua_tolstring(L, 2, &str_len);
    if(str == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::io::ArrayInputStream stream(str, str_len);
    if(message->ParseFromZeroCopyStream(&stream) == 0){
        LOG_LOG("parse fail\n");
        lua_pushboolean(L, false);
        return 1;
    }
    lua_pushboolean(L, true);
    return 1;
}
/*
 * @arg1 msg
 * @arg2 buf
 */
static int lmsg_serialize(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    if (message_lua->message == NULL) {
        LOG_LOG("null");
        return 0;
    }
    char *buf = (char *)lua_touserdata(L, 2);
    char * buf_end = (char *)message_lua->message->SerializeWithCachedSizesToArray((google::protobuf::uint8 *)buf);
    lua_pushinteger(L, buf_end - buf);
    return 1;
}

static int lmsg_bytesize(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    if (message_lua->message == NULL) {
        LOG_LOG("null");
        return 0;
    }
    lua_pushinteger(L, message_lua->message->ByteSize());
    return 1;
}

static int lmsg_isdirty(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    bool rt = (message_lua->dirty == 1);
    lua_pushboolean(L, rt);
    return 1;
}

//将message转为lua table
static void pb_load_tree(const google::protobuf::Message *message, lua_State* L){
    lua_newtable(L);
    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    const google::protobuf::Reflection* reflection = message->GetReflection();
    for(int i = 0; i < descriptor->field_count(); i++){
        const google::protobuf::FieldDescriptor *field = descriptor->field(i);
        if(field->is_repeated()){
            lua_pushstring(L, field->name().data());
            lua_newtable(L);
            int size = reflection->FieldSize(*message, field);
            for(int i = 0; i < size; i++){
                    if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE){
                        lua_pushnumber(L, i + 1);
                        const google::protobuf::Message &field_message =reflection->GetRepeatedMessage(*message, field, i);
                        pb_load_tree(&field_message, L);
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING){
                        lua_pushnumber(L, i + 1);
                        lua_pushstring(L, reflection->GetRepeatedString(*message, field, i).data());
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32){
                        lua_pushnumber(L, i + 1);
                        lua_pushnumber(L, reflection->GetRepeatedInt32(*message, field, i));
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32){
                        lua_pushnumber(L, i + 1);
                        lua_pushnumber(L, reflection->GetRepeatedUInt32(*message, field, i));
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT){
                        lua_pushnumber(L, i + 1);
                        lua_pushnumber(L, reflection->GetRepeatedFloat(*message, field, i));
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE){
                        lua_pushnumber(L, i + 1);
                         lua_pushnumber(L, reflection->GetRepeatedDouble(*message, field, i));
                        lua_settable(L, -3);
                    }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL){
                        lua_pushnumber(L, i + 1);
                        lua_pushboolean(L, reflection->GetRepeatedBool(*message, field, i));
                        lua_settable(L, -3);
                    }

            }
            lua_settable(L, -3);
        }else if(reflection->HasField(*message, field)){
            if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE){
                lua_pushstring(L, field->name().data());
                const google::protobuf::Message &field_message =reflection->GetMessage(*message, field, pblua_factory());
                pb_load_tree(&field_message, L);
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING){
                lua_pushstring(L, field->name().data());
                lua_pushstring(L, reflection->GetString(*message, field).data());
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32){
                lua_pushstring(L, field->name().data());
                lua_pushnumber(L, reflection->GetInt32(*message, field));
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32){
                lua_pushstring(L, field->name().data());
                 lua_pushnumber(L, reflection->GetUInt32(*message, field));
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT){
                lua_pushstring(L, field->name().data());
                 lua_pushnumber(L, reflection->GetFloat(*message, field));
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE){
                lua_pushstring(L, field->name().data());
                 lua_pushnumber(L, reflection->GetDouble(*message, field));
                lua_settable(L, -3);
            }else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL){
                lua_pushstring(L, field->name().data());
                lua_pushboolean(L, reflection->GetBool(*message, field));
                lua_settable(L, -3);
            }else{
                LOG_LOG("awfsaf");
            }
       }
   }
}

//将message转为lua table
static int lmsg_totable(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    //遍历message, 生成table
    pb_load_tree(message, L);
    return 1;
}

static int lmsg_debug_string(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    lua_pushstring(L, message->DebugString().data());
    return 1;
}

static int lmsg_setdirty(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    int flag = (int)lua_tonumber(L, 2);
    message_lua->dirty = flag;
    lua_pushboolean(L, true);
    return 1;
}

static int lmsg_mergefrom(lua_State *L){
    LuaMessage *message_lua1 = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message1 = message_lua1->message;
    if(message1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    LuaMessage *message_lua2 = (LuaMessage *)luaL_checkudata(L, 2, "LuaMessage");
    if(message_lua2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message2 = message_lua2->message;
    if(message2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    //pb刷新后, descriptor就对不上了
    if(message1->GetDescriptor() != message2->GetDescriptor()){
        LOG_LOG("copy %s from %s", message1->GetTypeName().data(), message2->GetTypeName().data());
        return 0;
    }
    /*
    if(strcmp(message1->GetTypeName().data(), message2->GetTypeName().data())){
        LOG_LOG("copy %s from %s", message1->GetTypeName().data(), message2->GetTypeName().data());
        return 0;
    }
    */
    message_lua1->root_message->dirty = 1;
    message1->MergeFrom(*message2);
    lua_pushboolean(L, true);
    return 1;
}

static int lmsg_repeated_merge_from(lua_State *L){
    RepeatedField *repeated_field1 = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    LuaMessage *message_lua1 = repeated_field1->message;
    if(message_lua1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field1 = repeated_field1->field;
    google::protobuf::Message *message1 = message_lua1->message;
    if(message1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection1 = message1->GetReflection();
    if(reflection1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    RepeatedField *repeated_field2 = (RepeatedField *)luaL_checkudata(L, 2, "RepeatedField");
    if(repeated_field2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    LuaMessage *message_lua2 = repeated_field2->message;
    if(message_lua2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field2 = repeated_field2->field;
    google::protobuf::Message *message2 = message_lua2->message;
    if(message2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection2 = message2->GetReflection();
    if(reflection2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    message_lua1->root_message->dirty = 1;
    int count = reflection2->FieldSize(*message2, field2);
    for(int i = 0; i < count; i++){
        if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
            reflection1->AddString(message1, field1, reflection2->GetRepeatedString(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
            reflection1->AddInt32(message1, field1, reflection2->GetRepeatedInt32(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
            reflection1->AddUInt32(message1, field1, reflection2->GetRepeatedUInt32(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT){
            reflection1->AddFloat(message1, field1, reflection2->GetRepeatedFloat(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE){
            reflection1->AddDouble(message1, field1, reflection2->GetRepeatedDouble(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL){
            reflection1->AddBool(message1, field1, reflection2->GetRepeatedBool(*message2, field2, i));
        }else if(field1->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
            google::protobuf::Message *message_new1 = reflection1->AddMessage(message1, field1);
            const google::protobuf::Message *message_new2 = &(reflection2->GetRepeatedMessage(*message2, field2, i));
            if(message_new1 == NULL || message_new2 == NULL){
                LOG_LOG("null");
                break;
            }
            message_new1->CopyFrom(*message_new2);
        }else{
            LOG_LOG("add fail file type :%d", field1->cpp_type());
        }
    }
    lua_pushboolean(L, true);
    return 1;
}


static int lmsg_copyfrom(lua_State *L){
    LuaMessage *message_lua1 = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message1 = message_lua1->message;
    if(message1 == NULL){
        LOG_LOG("null");
        return 0;
    }
    LuaMessage *message_lua2 = (LuaMessage *)luaL_checkudata(L, 2, "LuaMessage");
    if(message_lua2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message2 = message_lua2->message;
    if(message2 == NULL){
        LOG_LOG("null");
        return 0;
    }
    //pb刷新后, descriptor就对不上了
    if(message1->GetDescriptor() != message2->GetDescriptor()){
        LOG_LOG("copy %s from %s", message1->GetTypeName().data(), message2->GetTypeName().data());
        return 0;
    }
    /*
    if(strcmp(message1->GetTypeName().data(), message2->GetTypeName().data())){
        LOG_LOG("copy %s from %s", message1->GetTypeName().data(), message2->GetTypeName().data());
        return 0;
    }
    */
    message_lua1->root_message->dirty = 1;
    message1->CopyFrom(*message2);
    lua_pushboolean(L, true);
    return 1;
}

static int lmsg_msgname(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    lua_pushstring(L, message->GetTypeName().data());
    return 1;
}

static int lmsg_tostring(lua_State *L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    std::string str;
    if(message->SerializeToString(&str) == false){
        lua_pushnil(L);
        return 1; 
    }
    lua_pushlstring(L, str.data(), str.size());
    return 1;
}

static int lmsg_get(lua_State* L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    const char *field_name = lua_tostring(L, 2);
    google::protobuf::Message *message = message_lua->message;
    //LOG_LOG(field_name);
    if(message == NULL){
        return 0;
    }
    //LOG_LOG(field_name);
    //LOG_LOG("%d", message_lua);
    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    //LOG_LOG(field_name);
    if(descriptor == NULL){
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(field_name);
    if(field == NULL){
        if(strcmp(field_name, "tostring") == 0){
            lua_pushcfunction(L, lmsg_tostring);
            return 1;
        } else if(strcmp(field_name, "msgname") == 0) {
            lua_pushcfunction(L, lmsg_msgname);
            return 1;
        } else if(strcmp(field_name, "parse_from_string") == 0) {
            lua_pushcfunction(L, lmsg_parse_from_string);
            return 1;
        } else if(strcmp(field_name, "parse_from_buf") == 0) {
            lua_pushcfunction(L, lmsg_parse_from_buf);
            return 1;
        } else if(strcmp(field_name, "debug_string") == 0) {
            lua_pushcfunction(L, lmsg_debug_string);
            return 1;
        } else if(strcmp(field_name, "bytesize") == 0) {
            lua_pushcfunction(L, lmsg_bytesize);
            return 1;
        } else if(strcmp(field_name, "totable") == 0) {
            lua_pushcfunction(L, lmsg_totable);
            return 1;
        } else if(strcmp(field_name, "serialize") == 0) {
            lua_pushcfunction(L, lmsg_serialize);
            return 1;
        } else if(strcmp(field_name, "isdirty") == 0) {
            lua_pushcfunction(L, lmsg_isdirty);
            return 1;
        } else if(strcmp(field_name, "setdirty") == 0) {
            lua_pushcfunction(L, lmsg_setdirty);
            return 1;
        } else if(strcmp(field_name, "copyfrom") == 0) {
            lua_pushcfunction(L, lmsg_copyfrom);
            return 1;
        }else {
            LOG_LOG("unknow field %s", field_name);
            return 0;
        }
    }
    if(field->is_repeated()){
        RepeatedField *repeated_field = (RepeatedField *)lua_newuserdata(L, sizeof(RepeatedField));
        if(repeated_field == NULL){
            LOG_LOG("null");
            return 0;
        }   
        repeated_field->message = message_lua;
        repeated_field->field = field;
        luaL_getmetatable(L, "RepeatedField");
        lua_setmetatable(L, -2);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
        google::protobuf::Message *message_new = reflection->MutableMessage(message, field, pblua_factory());
        if(message_new == NULL){
            LOG_LOG("null");
            return 0;
        }
        LuaMessage *message_embed = (LuaMessage *)lua_newuserdata(L, sizeof(LuaMessage));
        if(message_embed == NULL){
            LOG_LOG("null");
            return 0;
        }
        message_embed->message = message_new;
        message_embed->root_message = message_lua->root_message;
        message_embed->dirty = 0;
        luaL_getmetatable(L, "LuaMessage");
        lua_setmetatable(L, -2);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
        std::string str = reflection->GetString(*message, field);
        int str_len = str.size();
        lua_pushlstring(L, str.data(), str_len);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
        lua_pushinteger(L, reflection->GetInt32(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
        lua_pushnumber(L, reflection->GetUInt32(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT64){
        lua_pushnumber(L, reflection->GetInt64(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT64){
        lua_pushnumber(L, reflection->GetUInt64(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT){
        lua_pushnumber(L, reflection->GetFloat(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE){
        lua_pushnumber(L, reflection->GetDouble(*message, field));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL){
        lua_pushboolean(L, reflection->GetBool(*message, field));
        return 1;
    }else{
        LOG_LOG("wrong type %d", field->type());
    }
    return 0;
}


static int lmsg_set(lua_State* L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    const char *field_name = lua_tostring(L, 2);
    google::protobuf::Message *message = message_lua->message;
    if(message == NULL){
        LOG_LOG("message is null field is %s", field_name);
        return 0;
    }
    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    if(descriptor == NULL){
        LOG_LOG("descriptor is null %s", message->GetTypeName().data());
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("reflection is null %s", message->GetTypeName().data());
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(field_name);
    if(field == NULL){
        LOG_LOG("field is not exists %s.%s", message->GetTypeName().data(), field_name);
        return 0;
    }
    if(field->is_repeated()){
        LOG_LOG("field is repeated %s.%s", message->GetTypeName().data(), field_name);
        return 0;
    }
    if(message_lua->root_message != NULL){
        message_lua->root_message->dirty = 1;
    }
    if(lua_isnil(L, 3) && field->label() == google::protobuf::FieldDescriptor::LABEL_REQUIRED){
        LOG_LOG("%s is required, but nil", field->full_name().data());
        return 0;
    }
    if(lua_isnil(L, 3)){
        reflection->ClearField(message, field);
        lua_pushinteger(L, 1);
        return 1;
    }
    if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE && lua_isstring(L, 3)){
        size_t str_len = 0;
        const char *str = (const char *)lua_tolstring(L, 3, &str_len);
        google::protobuf::Message *message_new = reflection->MutableMessage(message, field, pblua_factory());
        if(message_new == NULL){
            return 0;
        }
        google::protobuf::io::ArrayInputStream stream(str, str_len);
        if(message_new->ParseFromZeroCopyStream(&stream) == 0){
            LOG_LOG("%s.%s parse fail", message->GetTypeName().data(), field_name);
            lua_pushboolean(L, false);
            return 1;
        }
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE && lua_isuserdata(L, 3)){
        google::protobuf::Message *message_field = reflection->MutableMessage(message, field, pblua_factory());
        if(message_field == NULL){
            return 0;
        }
        LuaMessage *message_new = (LuaMessage *)luaL_checkudata(L, 3, "LuaMessage");
        if(message_new == NULL || message_new->message == NULL){
            return 0;
        }
        //pb刷新后, descriptor就对不上了
        if(message_field->GetDescriptor() != message_new->message->GetDescriptor()){
            LOG_LOG("type is diffenert %s = %s", message_field->GetTypeName().data(), 
                message_new->message->GetTypeName().data());
            return 0;
        }
       /* 
        if(strcmp(message_field->GetTypeName().data(), message_new->message->GetTypeName().data())){
            LOG_LOG("type is diffenert %s = %s", message_field->GetTypeName().data(), 
                message_new->message->GetTypeName().data());
            return 0;
        }
        */
        message_field->CopyFrom(*message_new->message);
        lua_pushboolean(L, true);
        return 1;
    }
    else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING && lua_isstring(L, 3)){
        size_t str_len = 0;
        const char *c_str = (const char *)lua_tolstring(L, 3, &str_len);
        std::string str(c_str, str_len);
        reflection->SetString(message, field, str);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32 && lua_isnumber(L, 3)){
        int32_t val = (int32_t)lua_tonumber(L, 3);
        //对int优化, 如果等于默认值则clear
        if(field->has_default_value()
        && field->default_value_int32() == val){
            reflection->ClearField(message, field);
            lua_pushnumber(L, 1);
            return 1;
        }else{
            reflection->SetInt32(message, field, val);
            lua_pushnumber(L, 1);
            return 1;
        }
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32 && lua_isnumber(L, 3)){
        uint32_t val = (uint32_t)lua_tonumber(L, 3);
        reflection->SetUInt32(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT64 && lua_isnumber(L, 3)){
        int64_t val = (int64_t)lua_tonumber(L, 3);
        reflection->SetInt64(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT64 && lua_isnumber(L, 3)){
        uint64_t val = (uint64_t)lua_tonumber(L, 3);
        reflection->SetUInt64(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT && lua_isnumber(L, 3)){
        float val = (float)lua_tonumber(L, 3);
        reflection->SetFloat(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE && lua_isnumber(L, 3)){
        double val = (double)lua_tonumber(L, 3);
        reflection->SetDouble(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL && lua_isboolean(L, 3)){
        int val = (int)lua_toboolean(L, 3);
        reflection->SetBool(message, field, val);
        lua_pushnumber(L, 1);
        return 1;
    }
    return 0;
}

static int lmsg_delete(lua_State* L){
    LuaMessage *message_lua = (LuaMessage *)luaL_checkudata(L, 1, "LuaMessage");
    if(message_lua == NULL){
        return 0;
    }
    if(message_lua->root_message != message_lua){
        return 0;
    }
    google::protobuf::Message *message = message_lua->message;
    if(message != NULL){
        //LOG_LOG("...........delete msg %d...........", message_lua); 
        s_msg_count--;
        s_msg_delete_times++;
        message_lua->message = NULL;
        delete message;
    }
    return 0;
}

static int lmsgclean(lua_State* L){
    google::protobuf::compiler::DiskSourceTree *new_sourceTree = new google::protobuf::compiler::DiskSourceTree();
    if(new_sourceTree == NULL){
        LOG_LOG("alloc fail");
        return 0;
    }
    google::protobuf::compiler::Importer *new_importer = new google::protobuf::compiler::Importer(new_sourceTree, errorCollector);
    if(new_importer == NULL){
        LOG_LOG("alloc fail");
        return 0;
    }
    sourceTree = new_sourceTree;
    importer = new_importer;
    sourceTree->MapPath("", "");
    pool = importer->pool();
    lua_pushboolean(L, true);
    return 1;
}

static int lget_descriptor(lua_State* L){
    if(lua_isstring(L, 1)){
        const char *msg_name = (const char *)lua_tostring(L, 1);
        const google::protobuf::Descriptor* descriptor = pool->FindMessageTypeByName(msg_name);
        if(descriptor == NULL){
            LOG_LOG("can not find msg %s", msg_name);
            return 0;
        }
        lua_newtable(L);
        for(int i = 0; i < descriptor->field_count(); i++){
            const google::protobuf::FieldDescriptor *field = descriptor->field(i);
            lua_pushstring(L, field->name().data());
            lua_newtable(L);
            lua_pushstring(L, "type");
            if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32
            || field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
                lua_pushstring(L, "int");
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
                lua_pushstring(L, "string");
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
                lua_pushstring(L, "message");
            }
            else{
                lua_pushstring(L, "unknown");
            }
            lua_settable(L, -3);
            
            lua_pushstring(L, "number");
            lua_pushnumber(L, field->number());
            lua_settable(L, -3);
            if(field->has_default_value()){
                lua_pushstring(L, "default");
                if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
                    lua_pushinteger(L, field->default_value_int32());
                }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
                    lua_pushinteger(L, field->default_value_uint32());
                }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
                    lua_pushstring(L, field->default_value_string().data());
                }else{
                    lua_pushnil(L);
                }
                lua_settable(L, -3);
            }

            lua_settable(L, -3);
        }
        return 1;
    }else{
        return 0;
    }
}

static int lmsgnew(lua_State* L){
    if(lua_isstring(L, 1)){
        const char *msg_name = (const char *)lua_tostring(L, 1);
        google::protobuf::Message *message = pblua_load_msg(msg_name);
        if(message == NULL){
            return 0;
        }
        LuaMessage *message_lua = (LuaMessage *)lua_newuserdata(L, sizeof(LuaMessage));
        if(message_lua == NULL){
            delete message;
            return 0;
        }
        message_lua->message = message;
        message_lua->root_message = message_lua;
        message_lua->dirty = 0;
        luaL_getmetatable(L, "LuaMessage");
        lua_setmetatable(L, -2);
        return 1;
    }else{
        return 0;
    }
}

static int lmsg_repeated_iter(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    int index = (int)lua_tonumber(L, 2) + 1;
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(index <= 0 || index > reflection->FieldSize(*message, field)){
        lua_pushnil(L);
        return 1;
    }
    if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
        lua_pushnumber(L, index);
        LuaMessage *message_lua = (LuaMessage *)lua_newuserdata(L, sizeof(LuaMessage));
        if(message_lua == NULL){
            LOG_LOG("null");
            return 0;
        }
        const google::protobuf::Message *message_new = &(reflection->GetRepeatedMessage(*message, field, index - 1));
        if(message_new == NULL){
            LOG_LOG("null");
            return 0;
        }
        message_lua->message = (google::protobuf::Message *)message_new;
        message_lua->root_message = repeated_field->message->root_message;
        message_lua->dirty = 0;
        luaL_getmetatable(L, "LuaMessage");
        lua_setmetatable(L, -2);
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
        lua_pushnumber(L, index);
        lua_pushstring(L, reflection->GetRepeatedString(*message, field, index - 1).data());
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
        lua_pushnumber(L, index);
        lua_pushinteger(L, reflection->GetRepeatedInt32(*message, field, index - 1));
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
        lua_pushnumber(L, index);
        lua_pushnumber(L, reflection->GetRepeatedUInt32(*message, field, index - 1));
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT){
        lua_pushnumber(L, index);
        lua_pushnumber(L, reflection->GetRepeatedFloat(*message, field, index - 1));
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE){
        lua_pushnumber(L, index);
        lua_pushnumber(L, reflection->GetRepeatedDouble(*message, field, index - 1));
        return 2;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL){
        lua_pushnumber(L, index);
        lua_pushboolean(L, reflection->GetRepeatedBool(*message, field, index - 1));
        return 2;
    }
    return 0;
}

//遍历repeated数组
static int lmsg_repeated_pairs(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    lua_pushcfunction(L, lmsg_repeated_iter);
    lua_pushvalue(L, -2);
    lua_pushnumber(L, 0);
    return 3;
}

static int lmsg_repeated_add(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
        const char* val = (const char *)lua_tostring(L, 2);
        reflection->AddString(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
        int val = (int)lua_tointeger(L, 2);
        reflection->AddInt32(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
        unsigned int val = (unsigned int)lua_tonumber(L, 2);
        reflection->AddUInt32(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT){
        float val = (float)lua_tonumber(L, 2);
        reflection->AddFloat(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE){
        double val = (double)lua_tonumber(L, 2);
        reflection->AddDouble(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL){
        int val = (int)lua_toboolean(L, 2);
        reflection->AddBool(message, field, val);
        lua_pushboolean(L, true);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
        LuaMessage *message_lua = (LuaMessage *)lua_newuserdata(L, sizeof(LuaMessage));
        if(message_lua == NULL){
            LOG_LOG("null");
            return 0;
        }
        google::protobuf::Message *message_new = reflection->AddMessage(message, field);
        if(message_new == NULL){
            LOG_LOG("null");
            return 0;
        }
        message_lua->message = message_new;
        message_lua->root_message = repeated_field->message->root_message;
        message_lua->dirty = 0;
        luaL_getmetatable(L, "LuaMessage");
        lua_setmetatable(L, -2);
        return 1;
    }else{
        LOG_LOG("add fail file type :%d", field->cpp_type());
        return 0;
    }
}

/*
 * uids.set(1, 54480920)
 *
 */
static int lmsg_repeated_set(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    int index = (int)lua_tonumber(L, 2) - 1;
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(index < 0 || index >= reflection->FieldSize(*message, field)){
        return 0;
    }
    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING && lua_type(L, 3) == LUA_TSTRING){
        const char *str = (const char *)lua_tostring(L, 3);
        reflection->SetRepeatedString(message, field, index, str);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32 && lua_type(L, 3) == LUA_TNUMBER){
        int val = (int)lua_tonumber(L, 3);
        reflection->SetRepeatedInt32(message, field, index, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32 && lua_type(L, 3) == LUA_TNUMBER){
        unsigned int val = (unsigned int)lua_tonumber(L, 3);
        reflection->SetRepeatedUInt32(message, field, index, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT && lua_type(L, 3) == LUA_TNUMBER){
        float val = (float)lua_tonumber(L, 3);
        reflection->SetRepeatedFloat(message, field, index, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE && lua_type(L, 3) == LUA_TNUMBER){
        double val = (double)lua_tonumber(L, 3);
        reflection->SetRepeatedDouble(message, field, index, val);
        lua_pushnumber(L, 1);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL && lua_type(L, 3) == LUA_TBOOLEAN){
        int val = (int)lua_toboolean(L, 3);
        reflection->SetRepeatedBool(message, field, index, val);
        lua_pushnumber(L, 1);
        return 1;
    }else{
        LOG_LOG("fail  %s", field->full_name().data());
    }
    return 0;
}


/*
 * local pet = pets.get(1)
 *
 */
static int lmsg_repeated_get(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    int index = (int)lua_tonumber(L, 2) - 1;
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("field null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(index < 0 || index >= reflection->FieldSize(*message, field)){
        lua_pushnil(L);
        return 1;
    }
    if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){
        LuaMessage *message_lua = (LuaMessage *)lua_newuserdata(L, sizeof(LuaMessage));
        if(message_lua == NULL){
            LOG_LOG("null");
            return 0;
        }
        const google::protobuf::Message *message_new = &(reflection->GetRepeatedMessage(*message, field, index));
        if(message_new == NULL){
            LOG_LOG("null");
            return 0;
        }
        message_lua->message = (google::protobuf::Message *)message_new;
        message_lua->root_message = repeated_field->message->root_message;
        message_lua->dirty = 0;
        luaL_getmetatable(L, "LuaMessage");
        lua_setmetatable(L, -2);
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING){
        lua_pushstring(L, reflection->GetRepeatedString(*message, field, index).data());
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32){
        lua_pushinteger(L, reflection->GetRepeatedInt32(*message, field, index));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32){
        lua_pushnumber(L, reflection->GetRepeatedUInt32(*message, field, index));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT){
        lua_pushnumber(L, reflection->GetRepeatedFloat(*message, field, index));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE){
        lua_pushnumber(L, reflection->GetRepeatedDouble(*message, field, index));
        return 1;
    }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL){
        lua_pushboolean(L, reflection->GetRepeatedBool(*message, field, index));
        return 1;
    }
    return 1;
}


/*
 * 返回数组的大小 
 * pets:count()
 *
 */
static int lmsg_repeated_count(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    int count = reflection->FieldSize(*message, field);
    lua_pushnumber(L, count);
    return 1;
}

static int lmsg_repeated_remove_last(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }

    if(reflection->FieldSize(*message, field) <= 0){
        lua_pushinteger(L, 1);
        return 1;
    }
    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    reflection->RemoveLast(message, field);
    lua_pushinteger(L, 1);
    return 1;
}

/*
 * 删除某个元素
 */
static int lmsg_repeated_remove_at(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    int index = (int)lua_tonumber(L, 2) - 1;
    if(index < 0 || index >= reflection->FieldSize(*message, field)){
        return 0;
    }

    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    int count = reflection->FieldSize(*message, field) - index - 1;
    for(int i = 0; i < count; i++){
        reflection->SwapElements(message, field, i + index, i + index + 1);
    }
    reflection->RemoveLast(message, field);
    lua_pushinteger(L, 1);
    return 1;
}

/*
 * 删除某个元素
 * pets:remove(1)
 * ps:现在的实现是将最后一个元素, 替换为这个元素, 再将这个元素删除
 * 所以循环时, 要注意, 可以从最后一个元素开始循环
 */

static int lmsg_repeated_remove(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    int index = (int)lua_tonumber(L, 2) - 1;
    if(index < 0 || index >= reflection->FieldSize(*message, field)){
        return 0;
    }
    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    reflection->SwapElements(message, field, index, reflection->FieldSize(*message, field) - 1);
    reflection->RemoveLast(message, field);
    lua_pushinteger(L, 1);
    return 1;
}

static int lmsg_repeated_clear(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message == NULL){
        LOG_LOG("null");
        return 0;
    }
    google::protobuf::Message *message = repeated_field->message->message;
    if(message == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    if(field == NULL){
        LOG_LOG("null");
        return 0;
    }
    const google::protobuf::Reflection* reflection = message->GetReflection();
    if(reflection == NULL){
        LOG_LOG("null");
        return 0;
    }
    if(repeated_field->message->root_message != NULL){
        repeated_field->message->root_message->dirty = 1;
    }
    reflection->ClearField(message, field);
    lua_pushinteger(L, 1);
    return 1;
}



/*
 *  查找符合某一条件的集合数据
 *  local cursor = pets:find({uid=54680920})
 *  for k , pet in pairs(cursor) do
 *      print(pet.uid)
 *  end
 *  find({uid={'$gt'=11}, }
 *  find({'$or'={{uid={'$gt=1}, {'a'}})
 *  ('uid>11 and uname>"hello")
 */
static int lmsg_repeated_find(lua_State *L){
    RepeatedField *repeated_field = (RepeatedField *)luaL_checkudata(L, 1, "RepeatedField");
    if(repeated_field == NULL){
        return 0;
    }
    if(repeated_field->message == NULL){
        return 0;
    }
    google::protobuf::Message *message_parent = repeated_field->message->message;
    const google::protobuf::FieldDescriptor *field = repeated_field->field;
    const google::protobuf::Reflection* reflection = message_parent->GetReflection();
    //只能处理Message
    if(field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE){ 
        return 0;
    }
    //处理查询条件
    char  *query_lua_key[50]; //[lua key][lua value type][value value]
    int query_lua_type[50];
    char * query_lua_string[50];
    lua_Number query_lua_number[50];
    bool query_lua_bool[50];

    int count = 0;
    lua_pushnil(L);
    while(lua_next(L, 2)){
        printf("%s - %s\n",lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
        query_lua_key[count] = (char *)lua_tostring(L, -2);
        query_lua_type[count] = lua_type(L, -1);
        if(lua_type(L, -1) == LUA_TSTRING){
            query_lua_string[count] = (char *)lua_tostring(L, -1);
        }else if(lua_type(L, -1) == LUA_TNUMBER){
            query_lua_number[count] = lua_tonumber(L, -1);
        }else if(lua_type(L, -1) == LUA_TBOOLEAN){
            query_lua_bool[count] = (bool)lua_toboolean(L, -1);
        }else{
        }
        count++;
        lua_pop(L, 1);
    }
    //结果集
    //lua_newtable(L);
    int size = reflection->FieldSize(*message_parent, field);
    //循环每个元素 
    for(int i = 0; i < size; i++){
        const google::protobuf::Message &message = reflection->GetRepeatedMessage(*message_parent, field, i);
        const google::protobuf::Reflection* reflection = message.GetReflection();
        const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
        if(descriptor == NULL){
            continue;
        }
        int is_find = 1;
        //循环每个属性
        for(int k = 0; k < count; k++){
            const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByName(query_lua_key[k]);
            if(field == NULL){
                //fail
                is_find = 0;
                break;
            }
            if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING
            && query_lua_type[k] == LUA_TSTRING){
                std::string value = reflection->GetString(message, field);
                int ir = strcmp(value.data(), (char *)query_lua_string[k]);
                if(ir != 0){is_find = 0;break;}
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32
            && query_lua_type[k] == LUA_TNUMBER){
                int value = reflection->GetInt32(message, field);
                int ir = value == (int)query_lua_number[k];
                if(ir != 0){is_find = 0;break;}
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32
            && query_lua_type[k] == LUA_TNUMBER){
                unsigned int value = reflection->GetUInt32(message, field);
                int ir = value == (unsigned int)query_lua_number[k];
                if(ir != 0){is_find = 0;break;}
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT
            && query_lua_type[k] == LUA_TNUMBER){
                float value = reflection->GetFloat(message, field);
                int ir = value == (float)query_lua_number[k];
                if(ir != 0){is_find = 0;break;}
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE
            && query_lua_type[k] == LUA_TNUMBER){
                double value = reflection->GetDouble(message, field);
                int ir = value == (double)query_lua_number[k];
                if(ir != 0){is_find = 0;break;}
            }else if(field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_BOOL
            && query_lua_type[k] == LUA_TNUMBER){
                bool value = reflection->GetBool(message, field);
                int ir = value == (bool)query_lua_bool[k];
                if(ir != 0){is_find = 0;break;}
            }
        }
    }
    return 0;
}

int luninit(lua_State *L){
	google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

static luaL_Reg lua_lib[] = 
{
    {"import", limport},
    {"mappath", lmappath},
    {"msgnew", lmsgnew},
    {"msgclean", lmsgclean},
    {"get_descriptor", lget_descriptor},
    //msg
    {"debug_string", lmsg_debug_string},
    {"tostring", lmsg_tostring},
    {"msgname", lmsg_msgname},
    {"parse_from_string", lmsg_parse_from_string},
    {"parse_from_buf", lmsg_parse_from_buf},
    {"isdirty", lmsg_isdirty},
    {"setdirty", lmsg_setdirty},
    {"copyfrom", lmsg_copyfrom},
    {"totable", lmsg_totable},
    {"mergefrom", lmsg_mergefrom},
    {"bytesize", lmsg_bytesize},
    {"serialize", lmsg_serialize},
    {"test", ltest},

    {NULL, NULL}
};

int luaopen_pblua(lua_State *L)
{
	luaL_register(L, "Pblua", (luaL_Reg*)lua_lib);

    sourceTree = new google::protobuf::compiler::DiskSourceTree();
    errorCollector = new MyMultiFileErrorCollector();
    importer = new google::protobuf::compiler::Importer(sourceTree, errorCollector);
    sourceTree->MapPath("", "");
    pool = importer->pool();
    factory = new google::protobuf::DynamicMessageFactory();

    //lua_newtable(L);
    //lua_setglobal(L, "pbenum");

    lua_pushcfunction(L, lmsg_repeated_pairs);
    lua_setglobal(L, "pbpairs");

    luaL_newmetatable(L, "LuaMessage");
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lmsg_set);
    lua_settable(L, -3);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lmsg_get);
    lua_settable(L, -3);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, lmsg_delete);
    lua_settable(L, -3);
   
    luaL_newmetatable(L, "RepeatedField");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    lua_pushstring(L, "add");
    lua_pushcfunction(L, lmsg_repeated_add);
    lua_settable(L, -3);
    lua_pushstring(L, "get");
    lua_pushcfunction(L, lmsg_repeated_get);
    lua_settable(L, -3);
    lua_pushstring(L, "set");
    lua_pushcfunction(L, lmsg_repeated_set);
    lua_settable(L, -3);
    lua_pushstring(L, "count");
    lua_pushcfunction(L, lmsg_repeated_count);
    lua_settable(L, -3);
    lua_pushstring(L, "remove");
    lua_pushcfunction(L, lmsg_repeated_remove);
    lua_settable(L, -3);
    lua_pushstring(L, "remove_at");
    lua_pushcfunction(L, lmsg_repeated_remove_at);
    lua_settable(L, -3);
    lua_pushstring(L, "remove_last");
    lua_pushcfunction(L, lmsg_repeated_remove_last);
    lua_settable(L, -3);
    lua_pushstring(L, "merge_from");
    lua_pushcfunction(L, lmsg_repeated_merge_from);
    lua_settable(L, -3);
    lua_pushstring(L, "clear");
    lua_pushcfunction(L, lmsg_repeated_clear);
    lua_settable(L, -3);
    lua_pushstring(L, "find");
    lua_pushcfunction(L, lmsg_repeated_find);
    lua_settable(L, -3);
    return 1;
}
