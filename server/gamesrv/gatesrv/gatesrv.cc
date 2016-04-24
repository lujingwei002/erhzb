#include "stdafx.h"

namespace Gatesrv
{

    static int listenfd = -1;
    static int sockfd_ = -1;

    int init()
    {
        return 0;
    }


    static void real_close(int sockfd, const char* reason)
    {
        aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE | AE_READABLE);
        Sendbuf::free(sockfd);
        Recvbuf::free(sockfd);
        close(sockfd);
        LOG("sockfd(%d) real close %s\n", sockfd, reason);
    }

    static void _ev_writable(struct aeEventLoop *eventLoop, int sockfd, void *clientData, int mask)
    {
        LOG("ev_writable\n");
        //发送数据
        for(;;)
        {
            int datalen = Sendbuf::datalen(sockfd);
            if (datalen <= 0)
            {
                LOG("delete write event\n");
                aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE);
                break;
            }
            char* buf = Sendbuf::get_read_ptr(sockfd);
            int ir = ::send(sockfd, buf, datalen, 0);
            LOG("real send %d\n", ir);
            if (ir > 0) 
            {
                Sendbuf::skip_read_ptr(sockfd, ir);
            } else if (ir == -1 && errno == EAGAIN) 
            {
                break;
            } else if(ir == -1) 
            {
                real_close(sockfd, "peer close");
                break;
            }
        }

    }

    int dispatch_frame(int sockfd, const char* data, unsigned short datalen)
    {
        char *dataptr = (char*)data;
        int sid = *(unsigned int *)dataptr;
        dataptr += 4;
        datalen -= 4;

        char *msgname = dataptr;
        unsigned short msgnamelen = 0;

        for (int i = 0; i < datalen; i++)
        {
            if (*(dataptr +i) == '*' || *(dataptr +i) == 0)
            {
                msgnamelen = i;
                dataptr += msgnamelen;
                datalen -= msgnamelen;
                break;
            }
        }
        if (msgnamelen == 0)
        {
            return 0;
        }


        printf("gatesrv recv a frame sid %d msgnamelen %d\n", sid, msgnamelen);

        if (*dataptr == '*')
        {
            //分发到lua处理
            static char funcname[64] = "Gatesrv.dispatch_json";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, sid);
            lua_pushlstring(Script::L, msgname, msgnamelen);
            lua_pushlstring(Script::L, dataptr + 1, datalen - 1);
            if (lua_pcall(Script::L, 3, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG("gatesrv.dispatch error %s\n", lua_tostring(Script::L, -1));
                }
            }
        }
        return 0;
    }

    int _decode_packet(int sockfd, const char* data, size_t datalen)
    {
        if (datalen < 2)
        {
            return 0;
        }
        unsigned short plen = *((unsigned short*)data);
        if (datalen < plen)
        {
            return 0;
        }
        char* body = (char*)data + sizeof(plen);
        dispatch_frame(sockfd, body, plen - sizeof(plen));
        return plen;
    }

    static void _ev_readable(struct aeEventLoop *eventLoop, int sockfd, void *clientData, int mask)
    {
        LOG("ev_readable\n");
        //接收数据
        for(;;)
        {
            char* wptr= Recvbuf::getwptr(sockfd);
            int buflen = Recvbuf::bufremain(sockfd);
            int ir = ::recv(sockfd, wptr, buflen, 0);
            LOG("sockfd(%d) real recv %d\n", sockfd, ir);
            if (ir == 0 || (ir == -1 && errno != EAGAIN))
            {
                real_close(sockfd, "peer close");
                break;
            }
            if (ir == -1 && errno == EAGAIN)
            {
                break;
            }
            Recvbuf::wskip(sockfd, ir);
            char* rptr = Recvbuf::getrptr(sockfd);
            int datalen = Recvbuf::datalen(sockfd);

            int packetlen = _decode_packet(sockfd, rptr, datalen);
            if (packetlen > 0)
            {
                Recvbuf::rskip(sockfd, packetlen);
                Recvbuf::buf2line(sockfd);
            }
            break;
        }
    }


    static void _ev_accept(struct aeEventLoop *eventLoop, int listenfd, void *clientData, int mask)
    {
        int sockfd;	
        int error;
        struct sockaddr_in addr;	
        socklen_t addrlen = sizeof(addr);	
        sockfd = ::accept(listenfd, (struct sockaddr*)&addr, &addrlen);
        if(sockfd < 0)
        {
            return;
        }
        error = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
        if (error < 0) 
        {
            return;
        }
        LOG("accept a new socket\n");
        Sendbuf::create(sockfd);
        Recvbuf::create(sockfd, 10240);
        aeCreateFileEvent(Net::loop, sockfd, AE_READABLE, _ev_readable, NULL);
        sockfd_ = sockfd;
    }



    int send(int sid, const void* data, unsigned short datalen)
    {
        if (sockfd_ == -1)
        {
            LOG("gatesrv not accept\n");
            return 0;
        }
        int plen = sizeof(datalen) + sizeof(sid) + datalen;
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG("gatesrv send %d to sockfd(%d)\n", plen, sockfd_);
        *(unsigned short*)buf = plen;
        *(unsigned int*)(buf + 2) = sid;
        memcpy(buf + sizeof(datalen) + sizeof(sid), data, datalen);
        Sendbuf::flush(sockfd_, buf, plen);
        aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
        return plen;
    }

    int listen(const char* host, unsigned short port)
    {
        if (listenfd != -1)
        {
            LOG("socket error\n");
            return 1;
        }
        int error;
        int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            LOG("socket error\n");
            return 1;
        }
        error = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
        if (error < 0)
        {
            ::close(sockfd);
            return 2;
        }
        struct sockaddr_in addr;
        bzero((void*)&addr, sizeof(addr));	
        addr.sin_family = AF_INET;	
        addr.sin_addr.s_addr = INADDR_ANY;	
        addr.sin_port = htons(port);	
        int reuse = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        error = ::bind(sockfd,(struct sockaddr *)&addr,sizeof(addr));
        if(error < 0){		
            ::close(sockfd);
            return 3;	
        }
        error = ::listen(sockfd, 5);	
        if(error < 0){
            ::close(sockfd);
            return 4;
        }
        listenfd = sockfd;
        aeCreateFileEvent(Net::loop, sockfd, AE_READABLE | AE_WRITABLE, _ev_accept, NULL);
        LOG("gatesrv listen success\n");
        return 0;
    }

};
