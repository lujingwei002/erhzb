#include "stdafx.h"


static int time_proc(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    //LOG_LOG("time_proc 1s\n");
    Dbclient::update();
    return 1000; 
}

int server_init(int argc, char **argv)
{
    LOG_LOG("server_init\n");
    Net::init();
    Gatesrv::init();
    Script::init();
    Log::closelevel(DEBUG);
    aeCreateTimeEvent(Net::loop, 1000, time_proc, NULL, NULL);
    return 0;
}


int server_loop()
{
    LOG_LOG("server_loop\n");
    Net::run();
    return 0;
}

int server_exit()
{
    LOG_LOG("server_exit\n");
    return 0;
}


int main(int argc, char **argv)
{
    server_init(argc, argv);
    server_loop();
    server_exit();
    return 0;
}
