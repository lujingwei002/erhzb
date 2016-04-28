#include "stdafx.h"

namespace Gameclient
{

    static bool is_connect_ = false;
    static int sockfd_ = -1;
    static char host_[128];
    static unsigned short port_;

    int init()
    {
        return 0;
    }

    bool is_connect()
    {
        return is_connect_;
    }

    static void real_close(int sockfd, const char* reason)
    {
        aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE | AE_READABLE);
        Sendbuf::free(sockfd);
        Recvbuf::free(sockfd);
        close(sockfd);
        sockfd_ = -1;
        is_connect_ = false;
        LOG_LOG("gameclient sockfd(%d) real close %s", sockfd, reason);
    }

    static void _ev_writable(struct aeEventLoop *eventLoop, int sockfd, void *clientData, int mask)
    {
        LOG_DEBUG("ev_writable");
        //发送数据
        for(;;)
        {
            int datalen = Sendbuf::datalen(sockfd);
            if (datalen <= 0)
            {
                LOG_DEBUG("delete write event");
                aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE);
                break;
            }
            char* buf = Sendbuf::get_read_ptr(sockfd);
            int ir = ::send(sockfd, buf, datalen, 0);
            LOG_DEBUG("real send %d", ir);
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
        int sid = *(unsigned int *)data;
        char* body = (char*)data + sizeof(unsigned int);
        unsigned short bodylen = datalen - sizeof(unsigned int);

        LOG_DEBUG("gameclient recv a frame sid %d data %s", sid, body);
        //分发到lua处理
        static char funcname[64] = "Gameclient.dispatch";
        Script::pushluafunction(funcname);
        lua_pushnumber(Script::L, sid);
        lua_pushlightuserdata(Script::L, (void*)body);
        lua_pushnumber(Script::L, bodylen);
        if (lua_pcall(Script::L, 3, 0, 0) != 0)
        {
            if (lua_isstring(Script::L, -1))
            {
                LOG_DEBUG("Gameclient.dispatch error %s", lua_tostring(Script::L, -1));
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
        LOG_DEBUG("ev_readable");
        //接收数据
        for(;;)
        {
            char* wptr= Recvbuf::getwptr(sockfd);
            int buflen = Recvbuf::bufremain(sockfd);
            int ir = ::recv(sockfd, wptr, buflen, 0);
            LOG_DEBUG("gameclient sockfd(%d) real recv %d", sockfd, ir);
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


    //打开会话
    int send_session_open(int sid)
    {
        if (!is_connect_)
        {
            LOG_LOG("gameclient is not connect");
            return 0;
        }
        unsigned char cmd = 2;
        int plen = sizeof(unsigned short) + sizeof(cmd) + sizeof(sid);
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG_LOG("gameclient send session open plen(%d) to sid(%d) sockfd(%d)", plen, sid, sockfd_);

        *(unsigned short*)buf = plen;
        buf += 2;

        *(unsigned char*)buf = cmd;
        buf += 1;

        *(unsigned int*)buf = sid;
        buf += 4;

        Sendbuf::flush(sockfd_, buf, plen);
        aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
        return plen;
    }

    //关闭会话
    int send_session_close(int sid)
    {
        if (!is_connect_)
        {
            LOG_LOG("gameclient is not connect");
            return 0;
        }
        unsigned char cmd = 3;
        int plen = sizeof(unsigned short) + sizeof(cmd) + sizeof(sid);
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG_LOG("gameclient send session close plen(%d) to sid(%d) sockfd(%d)", plen, sid, sockfd_);

        *(unsigned short*)buf = plen;
        buf += 2;

        *(unsigned char*)buf = cmd;
        buf += 1;

        *(unsigned int*)buf = sid;
        buf += 4;

        Sendbuf::flush(sockfd_, buf, plen);
        aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
        return plen;

    }
    
    int send_data(int sid, const void* data, unsigned short datalen)
    {
        if (!is_connect_)
        {
            LOG_LOG("gameclient is not connect");
            return 0;
        }
        unsigned char cmd = 1;
        int plen = sizeof(unsigned short) + sizeof(cmd) + sizeof(sid) + datalen;
        //插入到缓冲区
        char* buf = Sendbuf::alloc(sockfd_, plen);
        if (!buf)
        {
            return 0;
        }
        LOG_LOG("gameclient send data %d to sockfd(%d)", plen, sockfd_);

        *(unsigned short*)buf = plen;
        buf += 2;

        *(unsigned char*)buf = cmd;
        buf += 1;

        *(unsigned int*)buf = sid;
        buf += 4;

        memcpy(buf, data, datalen);
        buf += datalen;

        Sendbuf::flush(sockfd_, buf, plen);
        aeCreateFileEvent(Net::loop, sockfd_, AE_WRITABLE, _ev_writable, NULL);
        return plen;
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
                LOG_LOG("connect socket error");
                return 1;
            }
            error = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
            if (error < 0)
            {
                LOG_LOG("connect fcntl error");
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
                LOG_LOG("gameclient reconnect success");
                is_connect_ = true;
                Sendbuf::create(sockfd_);
                Recvbuf::create(sockfd_, 1024);
                aeCreateFileEvent(Net::loop, sockfd_, AE_READABLE, _ev_readable, NULL);
            } else
            {
//                LOG_DEBUG("gameclient reconnect error(%d) errno(%d)", error, errno);
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
