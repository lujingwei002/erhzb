

//tolua_begin
namespace Gamesrv
{
    int init();
    int listen(const char* host, unsigned short port);
    int post(lua_State* L);
};
//tolua_end
