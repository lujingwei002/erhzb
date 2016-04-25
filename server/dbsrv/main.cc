#include "stdafx.h"


int server_init(int argc, char **argv)
{
    LOG_LOG("server_init\n");
    Net::init();
    Gamesrv::init();
    Script::init();
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
