
//tolua_begin
namespace Sqlconn
{
    bool connect(const char*host, const char* dbname, const char* user, const char* passwd);
    bool command(const char* str);
    int select(lua_State *L);
    int escape(lua_State* L);
    int affectedrows();
    void update();
};
//tolua_end
