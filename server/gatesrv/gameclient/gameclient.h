#

//tolua_begin
namespace Gameclient
{
    int init();
    int connect(const char* host, unsigned short port);
    /*
     * sid:会话id
     * data:数据
     * datalen:数据长度
     */
    int send(int sid, const void* data, unsigned short datalen);
    int update();
};
//tolua_endinclude "stdafx.h"


