


//tolua_begin
namespace Websocket 
{

    int init();
    /*
     * 侦听
     */
    int listen(const char* host, unsigned short port);

    int send_string_frame(int sid, const char* str);
    int send_binary_frame(int sid, const void* data, unsigned short datalen);

};
//tolua_end
