#include "stdafx.h"


static int time_proc(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    //LOG_LOG("time_proc 1s\n");
    Gameclient::update();
    return 1000; 
}

int server_init(int argc, char **argv)
{
    LOG_LOG("server_init\n");
    Net::init();
    Websocket::init();
    Script::init();
    aeCreateTimeEvent(Net::loop, 1000, time_proc, NULL, NULL);
   // Log::closelevel(DEBUG);
    //Daemon::init();
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
