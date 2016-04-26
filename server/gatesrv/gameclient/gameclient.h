#

//tolua_begin
namespace Gameclient
{
    int init();
    int connect(const char* host, unsigned short port);
    bool is_connect();
    /*
     * sid:会话id
     * data:数据
     * datalen:数据长度
     */
    int send_data(int sid, const void* data, unsigned short datalen);
    int send_session_open(int sid);
    int send_session_close(int sid);
    int update();
};
//tolua_endinclude "stdafx.h"


