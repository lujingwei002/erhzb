#include "stdafx.h"


namespace Net
{
    aeEventLoop* loop;
    int init()
    {
        loop = aeCreateEventLoop(10240);
        return 0;
    }

    int run()
    {
        aeMain(loop);
        return 0;
    }
}
