#include "stdafx.h"
#include <signal.h>
namespace Daemon
{
    int init()
    {
        int pid;
        pid = fork();
        if(pid){
            exit(0);
        }else if(pid < 0){
            LOG_ERROR("fork error");
            exit(1);
        }
        /*  
            setsid();
            pid = fork();
            if(pid){
            exit(0);
            }else if(pid < 0){
            LOG_ERROR("fork error");
            exit(1);
            }
            */
        //ps, 关了就不要写
        /*for(i = 0; i <=2; i++){
          close(i);
          }*/
        //忽略ctrl-c
        //signal(SIGINT, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        //往关闭的socket写数据
        signal(SIGPIPE, SIG_IGN);
        //google protobuf出错时候会出这个
        signal(SIGABRT, SIG_IGN);
        //ctrl-c信号
        signal(SIGINT, SIG_IGN);
        return 0;
    }

};
