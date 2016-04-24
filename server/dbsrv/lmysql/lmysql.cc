#include "stdafx.h"


static int lconnect(lua_State *L){
    if(lua_gettop(L) == 4 && lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) && lua_isstring(L, 4)){
        const char *host = lua_tostring(L, 1);
        const char *dbname = lua_tostring(L, 2);
        const char *user = lua_tostring(L, 3);
        const char *passwd = lua_tostring(L, 4);
        my_bool b = 1;
        MYSQL *conn = mysql_init(NULL);
        mysql_options(conn, MYSQL_OPT_RECONNECT, &b);
        conn = mysql_real_connect(conn, host, user, passwd, dbname, 0, NULL, MYSQL_OPT_RECONNECT);
        if(conn == NULL){
            LOG("connect fail %s\n", mysql_error(conn));
            return 0;
        }
        mysql_query(conn, "set names utf8");
        //mysql_options(mysql->conn, MYSQL_SET_CHARSET_NAME, "utf8");
        lua_pushlightuserdata(L, conn);
        return 1;
    }
    return 0;
}

static int lcommand(lua_State *L){
    if(lua_gettop(L) == 2 && lua_isstring(L, 2)){
        MYSQL *conn = (MYSQL *)lua_touserdata(L, 1);
        if(conn == NULL){
            LOG("checkuserdata fail");
            lua_pushboolean(L, 0);
            return 1;
        }
        mysql_query(conn, "set names utf8");
        const char *command = lua_tostring(L, 2);
        if(mysql_query(conn, command) != 0){
            LOG("%s\n", mysql_error(conn));
            lua_pushboolean(L, 0);
            lua_pushstring(L, mysql_error(conn));
            return 2;
        }
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushboolean(L, 0);
    return 1;
}

static int laffectedrows(lua_State *L){
    if(lua_gettop(L) == 1){
        MYSQL *conn = (MYSQL *)lua_touserdata(L, 1);
        lua_pushnumber(L, mysql_affected_rows(conn));
        return 1;
    }
    lua_pushnumber(L, 0);
    return 1;
}

static int lselect(lua_State *L){
    int i;
    if(lua_gettop(L) == 2 && lua_isstring(L, 2)){
        MYSQL_RES *result;
        MYSQL_ROW row;
        MYSQL_FIELD *fields;
        int index = 1;
        MYSQL *conn = (MYSQL *)lua_touserdata(L, 1);
        if(conn == NULL){
            LOG("disconnect");
            return 0;
        }
        const char *command = lua_tostring(L, 2);
        mysql_query(conn, "set names utf8");
        mysql_query(conn, command);
        result = mysql_store_result(conn);
        if(result == NULL){
            LOG("select fail %d %s command %s", mysql_errno(conn), mysql_error(conn), command);
            return 0;
        }
        lua_newtable(L);
        while(result != NULL){
            int num_fields = mysql_num_fields(result);
            fields = mysql_fetch_fields(result);
            if(fields == NULL){
                mysql_free_result(result);
                LOG("select fail fetch_fields command:%s", command);
                break;
            }
            while ((row = mysql_fetch_row(result))){
                unsigned long *lengths = mysql_fetch_lengths(result);
                lua_pushnumber(L, index);
                lua_newtable(L);
                index++;
                for(i = 0; i < num_fields; i++){
                    lua_pushstring(L, fields[i].name);
                    if(row[i] != NULL){
                        if(fields[i].type == FIELD_TYPE_STRING
                            || fields[i].type == FIELD_TYPE_VAR_STRING
                            || fields[i].type == FIELD_TYPE_TINY_BLOB
                            || fields[i].type == FIELD_TYPE_MEDIUM_BLOB
                            || fields[i].type == FIELD_TYPE_LONG_BLOB
                            || fields[i].type == FIELD_TYPE_BLOB){
                            lua_pushlstring(L, row[i], lengths[i]);
                        }else if(fields[i].type == FIELD_TYPE_FLOAT
                            || fields[i].type == FIELD_TYPE_DOUBLE
                        ){
                            lua_pushnumber(L, atof(row[i]));
                        }
                        else{
                            lua_pushnumber(L, atoi(row[i]));
                        }
                    }else{
                        lua_pushnil(L);
                    }
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);
            }
            mysql_free_result(result);
            result = NULL;
            //是否还有结果集
            if(mysql_next_result(conn) == 0){
                result = mysql_store_result(conn);
            }
        }
        return 1;
    }
    return 0;
}


static char *escape_buf;
static size_t escape_buf_len;

static int lrealescapestring(lua_State *L){
    if(lua_gettop(L) == 2){
        MYSQL *conn = (MYSQL *)lua_touserdata(L, 1);
        if(conn == NULL){
            LOG("disconnect");
            return 0;
        }
        size_t str_len;
        const char *str = (const char *)lua_tolstring(L, 2, &str_len);
        if(str_len == 0){
            lua_pushstring(L, "");
            return 1;
        }
        if(escape_buf_len < str_len * 2){
            if(escape_buf != NULL){
                escape_buf_len = 0;
                free(escape_buf);
            }
            escape_buf = (char *)malloc(str_len * 2);
            if(escape_buf == NULL){
                LOG("malloc fail");
                return 0;
            }
            escape_buf_len = str_len * 2;
        }
        int len = mysql_real_escape_string(conn, escape_buf, str, str_len);
        lua_pushlstring(L, escape_buf, len);
        return 1;
    }
    return 0;
}


static int lclose(lua_State *L){
    if(lua_gettop(L) == 1){
        MYSQL *conn = (MYSQL *)lua_touserdata(L, 1);
        if(!conn){
            LOG("disconnect");
            return 0;
        }
        mysql_close(conn);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

/*
 *  local conn = Mysql.connect(ip, dbname, user, password)
 *  Mysql.command(conn, 'xx')
 *  Mysql.select(conn, 'xxx')
 *  Mysql.close(conn)
 *
 */
static luaL_Reg lua_lib[] = {
    {"connect", lconnect},
    {"select", lselect},
    {"command", lcommand},
    {"affectedrows", laffectedrows},
    {"escape", lrealescapestring},
    {"close", lclose},
    {NULL, NULL}
};

int luaopen_mysql(lua_State *L){
	luaL_register(L, "Mysql", (luaL_Reg*)lua_lib);
    return 1;
}
