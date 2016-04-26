

//tolua_begin
namespace Gatesrv
{
    int init();
    int listen(const char* host, unsigned short port);
    int send(int sid, const void* data, unsigned short datalen);
    int send_proto(lua_State* L);
};
//tolua_end
