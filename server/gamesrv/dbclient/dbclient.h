

//tolua_begin
namespace Dbclient
{
    int init();
    int connect(const char* host, unsigned short port);
    int send(int sockfd, const char* data, unsigned short datalen);
    int post(lua_State* L);
    int update();
    bool is_connect();
};
//tolua_end
