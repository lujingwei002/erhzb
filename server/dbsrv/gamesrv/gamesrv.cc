#include "stdafx.h"

namespace Gamesrv
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

    int dispatch_frame(int sockfd, const char* data, size_t datalen)
    {
        printf("recv a frame:%s\n", data);
        char *buf = (char *)data;

        unsigned short action_len = *(unsigned short*)buf;
        buf += 2;

        char *action = buf;
        buf += action_len;

        char c = buf[0];
        buf[0] = 0;
        Script::pushluafunction(action);
        //printf("safsafsafasfsfasaf %d %s\n", rt, action);
        buf[0] = c;

        unsigned short arg_num = *(unsigned short*)buf;
        buf += 2;

        //printf("safsafsafasfsfasaf %d\n", arg_num);
        for (int i = 0; i < arg_num; i++)
        {
            int type = *(unsigned char*)buf;
            buf += 1;
            if (type == LUA_TNUMBER)
            {
                int val = *(int *)buf;
                buf += 4;
                lua_pushnumber(Script::L, val);
            } else if(type == LUA_TSTRING)
            {
                unsigned short str_len = *(unsigned short*)buf;
                buf += 2;
                lua_pushlstring(Script::L, buf, str_len);
                buf += str_len;
            } else if(type == LUA_TNIL)
            {
                lua_pushnil(Script::L);
            }

        }
        //分发到lua处理
        if (lua_pcall(Script::L, arg_num, 0, 0) != 0)
        {
            if (lua_isstring(Script::L, -1))
            {
                LOG("gamesrv.dispatch error %s\n", lua_tostring(Script::L, -1));
            }
        }
        lua_pop(Script::L, lua_gettop(Script::L));
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
        sockfd_ = sockfd;
        aeCreateFileEvent(Net::loop, sockfd, AE_READABLE, _ev_readable, NULL);
    }

    //底层的SEND接口
    static int _real_send(int sockfd, const void* data, unsigned short datalen)
    {
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd, datalen + sizeof(unsigned short));
        if (!buf)
        {
            return 0;
        }
        LOG("send %d to sockfd(%d)\n", datalen, sockfd);
        *(unsigned short*)buf = datalen + sizeof(unsigned short);
        memcpy(buf + sizeof(unsigned short), data, datalen);
        Sendbuf::flush(sockfd, buf, datalen + sizeof(unsigned short));
        aeCreateFileEvent(Net::loop, sockfd, AE_WRITABLE, _ev_writable, NULL);
        return datalen;
    }

    int post(lua_State* L)
    {
        if (sockfd_ == -1)
        {
            return 0;
        }
        char *action = NULL;
        size_t action_len = 0;
        if (!lua_isstring(L, 1))
        {
            return 0;
        }
        action = (char *)lua_tolstring(L, 1, &action_len);
        int plen = 2 + sizeof(unsigned short) + action_len + sizeof(unsigned short);
        for (int i = 2; i <= lua_gettop(L); i++)
        {
            int type = lua_type(L, i);
            if (type == LUA_TNUMBER)
            {
                plen += (1 + 4); 
            } else if(type == LUA_TSTRING)
            {
                size_t str_len;
                lua_tolstring(L, i, &str_len);
                plen += (1 + 2 + str_len);
            } else if(type == LUA_TNIL)
            {
                plen += 1; 
            }
        }
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG("dbclient send %d to sockfd(%d)\n", plen, sockfd_);

        //写包长
        *(unsigned short*)buf = plen;
        buf += 2;

        //写方法名
        *(unsigned short*)buf = action_len;
        buf += 2;
        memcpy(buf, action, action_len);
        buf += action_len;

        //写参数个数
        *(unsigned short*)buf = lua_gettop(L) - 1;
        buf += 2;
        
        for (int i = 2; i <= lua_gettop(L); i++)
        {
            int type = lua_type(L, i);
            *(unsigned char*)buf = type;
            buf += 1;
            if (type == LUA_TNUMBER)
            {
                *(int*)buf = (int)lua_tonumber(L, i);
                buf += 4;
            } else if(type == LUA_TSTRING)
            {
                size_t str_len;
                char* str = (char *)lua_tolstring(L, i, &str_len);
                *(unsigned short*)buf = str_len;
                buf += 2;
                memcpy(buf, str, str_len);
                buf += str_len;
            } else if(type == LUA_TNIL)
            {
            }
        }
        Sendbuf::flush(sockfd_, buf, plen);
        aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
        return 0;
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
        LOG("dbsrvlisten success\n");
        return 0;
    }

};
