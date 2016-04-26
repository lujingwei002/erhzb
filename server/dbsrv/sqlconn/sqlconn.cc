#include "stdafx.h"

#include <mysql.h>
#include <stdlib.h>

namespace Sqlconn
{
    static MYSQL* conn_;

    void update()
    {
        if(mysql_ping(conn_) != 0)
        {
            LOG_ERROR("%s", mysql_error(conn_));
            return;
        }
    }

    bool connect(const char*host, const char* dbname, const char* user, const char* passwd)
    {
        if (conn_ != NULL)
        {
            return false;
        }
        my_bool b = 1;
        conn_ = mysql_init(NULL);
        conn_ = mysql_real_connect(conn_, host, user, passwd, dbname, 0, NULL, MYSQL_OPT_RECONNECT);
        if (conn_ == NULL)
        {
            LOG_LOG("connect fail %s\n", mysql_error(conn_));
            return false;
        }
        mysql_options(conn_, MYSQL_OPT_RECONNECT, &b);
        mysql_query(conn_, "set names utf8");
        LOG_LOG("connect database success")
        return true;
    }


    static char *escape_buf;
    static size_t escape_buf_len;
    int escape(lua_State* L)
    {
        if(conn_ == NULL)
        {
            LOG_LOG("disconnect");
            return 0;
        }
        size_t str_len;
        const char *str = (const char *)lua_tolstring(L, 1, &str_len);
        if(str_len == 0)
        {
            lua_pushstring(L, "");
            return 1;
        }
        if(escape_buf_len < str_len * 2)
        {
            if(escape_buf != NULL)
            {
                escape_buf_len = 0;
                free(escape_buf);
            }
            escape_buf = (char *)malloc(str_len * 2);
            if(escape_buf == NULL)
            {
                LOG_LOG("malloc fail");
                return 0;
            }
            escape_buf_len = str_len * 2;
        }
        int len = mysql_real_escape_string(conn_, escape_buf, str, str_len);
        lua_pushlstring(L, escape_buf, len);
        return 1;
    }

    bool command(const char* str)
    {
        if(conn_ == NULL)
        {
            LOG_LOG("mysql not connect");
            return false;
        }
        mysql_query(conn_, "set names utf8");
        if(mysql_query(conn_, str) != 0)
        {
            LOG_ERROR("%s\n", mysql_error(conn_));
            return false;
        }
        return true;
    }

    int affectedrows()
    {
        if(conn_ == NULL)
        {
            LOG_LOG("mysql not connect");
            return 0;
        }
        return mysql_affected_rows(conn_);
    }

    int select(lua_State *L)
    {
        if(conn_ == NULL)
        {
            LOG_LOG("mysql not connect");
            return 0;
        }
        int i;
        if(!lua_gettop(L) == 1 || !lua_isstring(L, 1))
        {
            return 0;
        }
        MYSQL_RES *result;
        MYSQL_ROW row;
        MYSQL_FIELD *fields;
        int index = 1;
        const char *command = lua_tostring(L, 1);
        mysql_query(conn_, "set names utf8");
        mysql_query(conn_, command);
        result = mysql_store_result(conn_);
        if(result == NULL)
        {
            LOG_LOG("select fail %d %s command %s", mysql_errno(conn_), mysql_error(conn_), command);
            return 0;
        }
        lua_newtable(L);
        while(result != NULL)
        {
            int num_fields = mysql_num_fields(result);
            fields = mysql_fetch_fields(result);
            if(fields == NULL)
            {
                mysql_free_result(result);
                LOG_LOG("select fail fetch_fields command:%s", command);
                break;
            }
            while ((row = mysql_fetch_row(result)))
            {
                unsigned long *lengths = mysql_fetch_lengths(result);
                lua_pushnumber(L, index);
                lua_newtable(L);
                index++;
                for(i = 0; i < num_fields; i++)
                {
                    lua_pushstring(L, fields[i].name);
                    if(row[i] != NULL)
                    {
                        if(fields[i].type == FIELD_TYPE_STRING
                                || fields[i].type == FIELD_TYPE_VAR_STRING
                                || fields[i].type == FIELD_TYPE_TINY_BLOB
                                || fields[i].type == FIELD_TYPE_MEDIUM_BLOB
                                || fields[i].type == FIELD_TYPE_LONG_BLOB
                                || fields[i].type == FIELD_TYPE_BLOB)
                        {
                            lua_pushlstring(L, row[i], lengths[i]);
                        }else if(fields[i].type == FIELD_TYPE_FLOAT
                                || fields[i].type == FIELD_TYPE_DOUBLE
                                )
                        {
                            lua_pushnumber(L, atof(row[i]));
                        }
                        else
                        {
                            lua_pushnumber(L, atoi(row[i]));
                        }
                    }else
                    {
                        lua_pushnil(L);
                    }
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);
            }
            mysql_free_result(result);
            result = NULL;
            //是否还有结果集
            if(mysql_next_result(conn_) == 0)
            {
                result = mysql_store_result(conn_);
            }
        }
        return 1;
    }
};
