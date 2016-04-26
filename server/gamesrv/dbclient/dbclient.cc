#include "stdafx.h"

namespace Dbclient
{

    static bool is_connect_ = false;
    static int sockfd_ = -1;
    static char host_[128];
    static unsigned short port_;


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
        sockfd_ = -1;
        is_connect_ = false;
        LOG_DEBUG("dbclient sockfd(%d) real close %s\n", sockfd, reason);
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
                LOG_DEBUG("gamesrv.dispatch error %s\n", lua_tostring(Script::L, -1));
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

    //底层的SEND接口
    static int _real_send(int sockfd, const void* data, unsigned short datalen)
    {
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd, datalen + sizeof(unsigned short));
        if (!buf)
        {
            return 0;
        }
        LOG_DEBUG("send %d to sockfd(%d)\n", datalen, sockfd);
        *(unsigned short*)buf = datalen + sizeof(unsigned short);
        memcpy(buf + sizeof(unsigned short), data, datalen);
        Sendbuf::flush(sockfd, buf, datalen + sizeof(unsigned short));
        aeCreateFileEvent(Net::loop, sockfd, AE_WRITABLE, _ev_writable, NULL);
        return datalen;
    }

    //底层的SEND接口
    int send(int sockfd, const char* data, unsigned short datalen)
    {
        if (!is_connect_)
        {
            return 1;
        }
        return _real_send(sockfd, data, datalen);
    }

    int post(lua_State* L)
    {
        if (!is_connect_)
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
        LOG_DEBUG("dbclient send %d to sockfd(%d)\n", plen, sockfd_);

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

    int connect(const char* host, unsigned short port)
    {
        strcpy(host_, host);
        port_ = port;
        return 0;
    }


    static int _real_connect()
    {
        int error;
        int sockfd;

        if (is_connect_)
        {
            return 1;
        }

        if (sockfd_ == -1)
        {
            sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                LOG_DEBUG("connect socket error\n");
                return 1;
            }
            error = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
            if (error < 0)
            {
                LOG_DEBUG("connect fcntl error\n");
                ::close(sockfd);
                return 1;
            }
            sockfd_ = sockfd;
        }

        if (sockfd_ != -1)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;	
            if(inet_addr(host_) != (in_addr_t)-1)
            {
                addr.sin_addr.s_addr = inet_addr(host_);   
            }else
            {
                struct hostent *hostent;
                hostent = gethostbyname(host_);
                if(hostent->h_addr_list[0] == NULL)
                {
                    return 1;
                }
                memcpy(&addr.sin_addr, (struct in_addr *)hostent->h_addr_list[0], sizeof(struct in_addr));
            } 
            addr.sin_port = htons(port_);        
            error = ::connect(sockfd_, (struct sockaddr *)&addr, sizeof(addr));
            if ((error == 0) || (error < 0 && errno == EISCONN))
            {
                LOG_DEBUG("dbclient reconnect success\n");
                is_connect_ = true;
                Sendbuf::create(sockfd_);
                Recvbuf::create(sockfd_, 1024);
                aeCreateFileEvent(Net::loop, sockfd_, AE_READABLE, _ev_readable, NULL);
                
                //通知连接成功
                static char funcname[64] = "Dbclient.on_connect";
                Script::pushluafunction(funcname);
                lua_pushnumber(Script::L, sockfd_);
                if (lua_pcall(Script::L, 1, 0, 0) != 0)
                {
                    Script::printluastack();
                }

            } else
            {
                //printf("dbclient reconnect error(%d) errno(%d)\n", error, errno);
            }
        }
        return 0;
    }

    int update()
    {
        if (!is_connect_ && host_[0])
        {
            _real_connect();
        }
        return 0;
    }
};
