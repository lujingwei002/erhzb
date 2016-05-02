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
        LOG_LOG("sockfd(%d) real close %s\n", sockfd, reason);
    }

    static void _ev_writable(struct aeEventLoop *eventLoop, int sockfd, void *clientData, int mask)
    {
        LOG_DEBUG("ev_writable\n");
        //发送数据
        for(;;)
        {
            int datalen = Sendbuf::datalen(sockfd);
            if (datalen <= 0)
            {
                LOG_DEBUG("delete write event\n");
                aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE);
                break;
            }
            char* buf = Sendbuf::get_read_ptr(sockfd);
            int ir = ::send(sockfd, buf, datalen, 0);
            LOG_DEBUG("real send %d\n", ir);
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
    int push_luamsgname(char *msgname, unsigned short msgnamelen)
    {
        int s = 0;
        char* ptr = msgname;
        for (int i = 0; i < msgnamelen; i++)
        {
            if (msgname[i] == '.')
            {
                //改成大写
                ptr[0] = ptr[0] - 32;
                lua_pushlstring(Script::L, ptr, i - s);
                ptr[0] = ptr[0] + 32;
                s = i + 1;
                ptr = (msgname + i + 1);
                break;
            }
        }
        lua_pushlstring(Script::L, ptr, msgnamelen - s);
        return 0;
    }

    int dispatch_frame(int sockfd, const char* data, unsigned short datalen)
    {
        char *dataptr = (char*)data;

        int cmd = *(unsigned char*)dataptr;
        dataptr += 1;
        datalen -= 1;

        int sid = *(unsigned int *)dataptr;
        dataptr += 4;
        datalen -= 4;

        if (cmd == 2)
        {
            //打开会话
            LOG_LOG("gatesrv recv a session open sid(%d)", sid);
            //分发到lua处理
            static char funcname[64] = "Gatesrv.on_session_open";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, sid);
            if (lua_pcall(Script::L, 1, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG_ERROR("gatesrv.on_session_open error %s\n", lua_tostring(Script::L, -1));
                }
            }
        }
        else if (cmd == 3)
        {
            //关闭会话
            LOG_LOG("gatesrv recv a session close sid(%d)", sid);
            //分发到lua处理
            static char funcname[64] = "Gatesrv.on_session_close";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, sid);
            if (lua_pcall(Script::L, 1, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG_DEBUG("gatesrv.on_session_open error %s\n", lua_tostring(Script::L, -1));
                }
            }
        }
        else if (cmd == 1)
        {
            //数据帧

            LOG_LOG("gatesrv recv a frame sid %d\n", sid);


            if (*dataptr == '*')
            {
                dataptr += 1;
                datalen -= 1;

                char *msgname = dataptr;

                unsigned short msgnamelen = 0;
                for (int i = 0; i < datalen; i++)
                {
                    if (*(dataptr +i) == '*')
                    {
                        msgnamelen = i;
                        break;
                    }
                }
                if (msgnamelen == 0)
                {
                    LOG_ERROR("frame is wrong");
                    return 0;
                }
                dataptr += (msgnamelen + 1);
                datalen -= (msgnamelen + 1);

                //分发到lua处理
                static char funcname[64] = "Gatesrv.dispatch_json";
                Script::pushluafunction(funcname);
                lua_pushnumber(Script::L, sid);
                lua_pushlstring(Script::L, msgname, msgnamelen);
                push_luamsgname(msgname, msgnamelen);
                lua_pushlstring(Script::L, dataptr, datalen);
                printf("aaaaaaa\n");
                if (lua_pcall(Script::L, 5, 0, 0) != 0)
                {
                    if (lua_isstring(Script::L, -1))
                    {
                        LOG_ERROR("gatesrv.dispatch error %s\n", lua_tostring(Script::L, -1));
                    }
                }
            } else if (*dataptr == 0)
            {
                dataptr += 1;
                datalen -= 1;

                unsigned short msgnamelen = *(unsigned short*)dataptr;
                //msgnamelen = ntohs(msgnamelen);
                dataptr += sizeof(unsigned short);
                datalen -= sizeof(unsigned short);

                char *msgname = dataptr;
                dataptr += msgnamelen;
                datalen -= msgnamelen;

                char c = msgname[msgnamelen];
                msgname[msgnamelen] = 0;
                LOG_LOG("gatesrv recv a frame from sid %d msgname %s msgnamelen %d datalen %d\n", sid, msgname, msgnamelen, datalen);
                msgname[msgnamelen] = c;


                //分发到lua处理
                static char funcname[64] = "Gatesrv.dispatch_proto";
                Script::pushluafunction(funcname);
                lua_pushnumber(Script::L, sid);
                lua_pushlstring(Script::L, msgname, msgnamelen);
                push_luamsgname(msgname, msgnamelen);
                lua_pushlstring(Script::L, dataptr, datalen);
                if (lua_pcall(Script::L, 5, 0, 0) != 0)
                {
                    if (lua_isstring(Script::L, -1))
                    {
                        LOG_ERROR("gatesrv.dispatch error %s\n", lua_tostring(Script::L, -1));
                    }
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
        LOG_DEBUG("ev_readable\n");
        //接收数据
        for(;;)
        {
            char* wptr= Recvbuf::getwptr(sockfd);
            int buflen = Recvbuf::bufremain(sockfd);
            int ir = ::recv(sockfd, wptr, buflen, 0);
            LOG_DEBUG("sockfd(%d) real recv %d\n", sockfd, ir);
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
        LOG_LOG("gatesrv accept a new socket sockfd(%d)\n", sockfd);
        Sendbuf::create(sockfd);
        Recvbuf::create(sockfd, 10240);
        aeCreateFileEvent(Net::loop, sockfd, AE_READABLE, _ev_readable, NULL);
        sockfd_ = sockfd;
    }


    int send_proto(lua_State* L)
    {
       int sid = (int)lua_tonumber(L, 1); 
       size_t msgnamelen = 0; 
       char* msgname = (char*)lua_tolstring(L, 2, &msgnamelen);
       
       size_t msgdatalen = 0;
       char* msgdata = (char*)lua_tolstring(L, 3, &msgdatalen);

       if (sockfd_ == -1)
       {
           LOG_DEBUG("gatesrv not accept\n");
           return 0;
       }

       int plen = sizeof(unsigned short) + sizeof(sid) + 1 + sizeof(unsigned short) + msgnamelen + msgdatalen;

       LOG_LOG("gatesrv send proto plen(%d) msgname(%s) msgdatalen(%d) to sid(%d)\n", plen, msgname, msgdatalen, sid);

       //插入到缓冲区
       char* buf = Sendbuf::alloc(sockfd_, plen);
       if (!buf)
       {
           return 0;
       }
       LOG_DEBUG("gatesrv send %d to sockfd(%d)\n", plen, sockfd_);

       *(unsigned short*)buf = plen;
       buf += 2;

       *(unsigned int*)buf = sid;
       buf += 4;

       *(unsigned char*)buf = 0;
       buf += 1;

       //*(unsigned short*)buf = htons(msgnamelen);
       *(unsigned short*)buf = msgnamelen;
       buf += 2;

       memcpy(buf, msgname, msgnamelen);
       buf += msgnamelen;

       memcpy(buf, msgdata, msgdatalen);
       buf += msgdatalen;

       Sendbuf::flush(sockfd_, buf, plen);
       aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
       return plen;

    }

    int send(int sid, const void* data, unsigned short datalen)
    {
        if (sockfd_ == -1)
        {
            LOG_ERROR("gatesrv not accept\n");
            return 0;
        }
        int plen = sizeof(datalen) + sizeof(sid) + datalen;
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG_DEBUG("gatesrv send %d to sockfd(%d)\n", plen, sockfd_);
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
            LOG_DEBUG("socket error\n");
            return 1;
        }
        int error;
        int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            LOG_DEBUG("socket error\n");
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
        LOG_DEBUG("gatesrv listen success\n");
        return 0;
    }

};
