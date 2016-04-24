

//tolua_begin
namespace Gatesrv
{
    int init();
    int listen(const char* host, unsigned short port);
    int send(int sid, const void* data, unsigned short datalen);
};
//tolua_end
