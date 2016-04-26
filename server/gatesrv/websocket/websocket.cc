#include "stdafx.h"

namespace Websocket
{
    typedef struct FrameHeader
    {
        unsigned char opcode:4;
        unsigned char rsv:3;
        unsigned char fin:1;
        unsigned char payload_len:7;
        unsigned char mask:1;
    }FrameHeader;

    typedef struct Session
    {
       int sockfd; 
       int had_shake;
       unsigned int sid;//会话id
    }Session;

    #define MAX_SESSION 10240
    Session session_array[MAX_SESSION]; 
    unsigned short sessionid_counter = 0;

    static int listenfd = -1;

    int send(int sockfd, const char* data);
    int _real_send(int sockfd, const void* data, size_t datalen);

    int init()
    {
        memset(session_array, sizeof(Session) * MAX_SESSION, 0);
        return 0;
    }

    static void session_close(int sockfd)
    {

    }

    static Session* session_find(unsigned int sid)
    {
        int sockfd = sid >> 16;
        if (sockfd <= 0 || sockfd >= MAX_SESSION)
        {
            return NULL;
        }
        Session* session = &session_array[sockfd];
        if (session->sid != sid)
        {
            return NULL;
        }
        return session;
    }

    static Session* session_get(int sockfd)
    {
        if (sockfd <= 0 || sockfd >= MAX_SESSION)
        {
            return NULL;
        }
        return &session_array[sockfd];
    }

    static void session_init(int sockfd)
    {
        Session* session = session_get(sockfd);
        if (!session)
        {
            LOG_DEBUG("session not found\n");
            return;
        }
        session->sockfd = sockfd;
        session->had_shake = 0;
        //初始化一个会话id
        session->sid = sockfd << 16 | sessionid_counter++;
        memset(session, sizeof(Session), 0);
    }

    static void real_close(int sockfd, const char* reason)
    {
        aeDeleteFileEvent(Net::loop, sockfd, AE_WRITABLE | AE_READABLE);
        Sendbuf::free(sockfd);
        Recvbuf::free(sockfd);
        close(sockfd);
        session_close(sockfd);
        LOG_LOG("sockfd(%d) real close %s\n", sockfd, reason);
    }

    static int _find_string(const char* text, size_t textlen, int start, const char* str, size_t len)
    {
        for (size_t i = start; i < textlen; i++)
        {
            if (!strncmp(text + i, str, len))
            {
                return i;
            }
        }
        return -1;
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

    static int dispatch_frame(Session* session, int opcode, const char* data, size_t datalen)
    {
        LOG_DEBUG("recv a frame opcode(%d) datalen(%d)\n", opcode, datalen);
        if (opcode == 8)
        {
            //分发到lua处理
            static char funcname[64] = "Websocket.on_session_close";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, session->sid);
            if (lua_pcall(Script::L, 1, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG_DEBUG("Websocket.dispatch error %s\n", lua_tostring(Script::L, -1));
                }
            }
            return 0;

        } else 
        {
            //分发到lua处理
            static char funcname[64] = "Websocket.dispatch";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, session->sid);
            lua_pushlightuserdata(Script::L, (void*)data);
            lua_pushnumber(Script::L, datalen);
            if (lua_pcall(Script::L, 3, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG_DEBUG("Websocket.dispatch error %s\n", lua_tostring(Script::L, -1));
                }
            }
            return 0;
        }
    }

    int combine_all_frame(Session* session, const char* data , size_t datalen)
    {
        char* framedata = (char*)data;
        char* combine_payload_data = (char*)data;
        unsigned long long packet_len = 0;
        unsigned long long total_len = 0;
        int opcode = 0;
        for(;;)
        {
            FrameHeader* frame_header = (FrameHeader*)framedata;
            int fin = frame_header->fin;
            opcode = frame_header->opcode;
            unsigned long long framelen = 2;
            unsigned long long real_payload_len = frame_header->payload_len;
            char *payload_data = 0;
            //解释负载长度
            if (frame_header->payload_len == 126)
            {
                framelen = 4;
            }
            else if (frame_header->payload_len == 127)
            {
                framelen = 9;
            }
            if (frame_header->mask == 1)
            {
                framelen += 4;
            }
            //负载
            payload_data = (char *)framedata + framelen;
            if (frame_header->payload_len == 126)
            {
                //2个字节的长度
                real_payload_len = ntohs(*((unsigned short*)(framedata + 2)));
            }
            else if (frame_header->payload_len == 127)
            {
                //暂时不支持8字节长度
                //8个字节的长度
                //TODO ntohl只能用于32位数
                //unsigned int* _real_len32 = (unsigned int*)(&real_payload_len);
                //_real_len32[0] = ntohl(*((unsigned int*)(framedata+ 4)));
                //_real_len32[1] = ntohl(*((unsigned int*)(framedata + 2)));
                real_payload_len = ntohl(*((unsigned long long*)(framedata+ 2)));

            }
            framelen += real_payload_len;

            //直接复制数据吧
            memcpy(combine_payload_data, payload_data, real_payload_len);
            total_len += framelen;
            packet_len += real_payload_len;
            framedata += framelen;
            combine_payload_data += real_payload_len;
            if (fin == 1)
            {
                break;
            }
        }
        *combine_payload_data = 0;
        dispatch_frame(session, opcode, data, packet_len);
        return total_len;
    }

    int decode_one_frame(int sockfd, const char* data, size_t datalen)
    {
        if(datalen < 2)
        {
            return 0;
        }
        FrameHeader* frame_header = (FrameHeader*)data;
        LOG_DEBUG("fin(%d) rsv(%d) mask(%d) opcode(%d) payload_len(%d)\n", frame_header->fin, frame_header->rsv, frame_header->mask, frame_header->opcode, frame_header->payload_len);
        LOG_DEBUG("c1 %d c2 %d\n", *(unsigned char *)data, *(unsigned char *)(data + 1));
        //接下来开始计算真实的长度
        unsigned long long framelen = 2;
        unsigned long long real_payload_len = frame_header->payload_len;
        //掩码
        unsigned char *mask = 0;
        //负载
        char *payload_data = 0;
        if (frame_header->payload_len == 126)
        {
            framelen = 4;
        }
        else if (frame_header->payload_len == 127)
        {
            framelen = 9;
        }
        if (frame_header->mask == 1)
        {
            mask = (unsigned char *)data + framelen;
            framelen += 4;
        }
        //负载
        payload_data = (char *)data + framelen;

        //测试数据长度
        if (datalen < framelen)
        {
            return 0;
        }

        //解释负载长度
        if (frame_header->payload_len == 126)
        {
            //2个字节的长度
            real_payload_len = ntohs(*((unsigned short*)(data + 2)));
        }
        else if (frame_header->payload_len == 127)
        {
            //暂时不支持8字节长度
            //TODO ntohl只能用于32位数
            //unsigned int* _real_len32 = (unsigned int*)(&real_payload_len);
            //_real_len32[0] = ntohl(*((unsigned int*)(data + 4)));
            //_real_len32[1] = ntohl(*((unsigned int*)(data + 2)));
            real_payload_len = ntohl(*((unsigned long long*)(data + 2)));
        }
        framelen += real_payload_len;
        //测试数据长度
        if (datalen < framelen)
        {
            return 0;
        }
        LOG_DEBUG("real_payload_len(%lld)\n", real_payload_len);
        //用掩码修改数据
        for (unsigned long long i = 0; i < real_payload_len; i++)
        {
            payload_data[i] = payload_data[i] ^ mask[i % 4];
        }
        //LOG_DEBUG("%s\n", data);
        //LOG_DEBUG("payload %s\n", payload_data);
        //是否最后一帧了
        return frame_header->fin;
    }

    static int _decode_packet(int sockfd, const char*data, size_t datalen)
    {
        Session* session = session_get(sockfd);
        if (!session)
        {
            LOG_DEBUG("session is nil\n");
            return 0;
        }
        //握手
        if (session->had_shake == 0)
        {
            LOG_DEBUG("%s\n", data);
            SHA1_CTX context; 
            SHA1Init(&context); 
            static unsigned char digest[20];
            static unsigned char key[128];
            int rnrn = _find_string(data, datalen, 0, "\r\n\r\n", 4);
            if (rnrn < 0)
            {
                return 0;
            }
            int key_start = _find_string(data, datalen, 0, "Sec-WebSocket-Key: ", 19);
            int key_end = _find_string(data, datalen, key_start, "\r\n", 2);
            if (key_start < 0 || key_end < 0)
            {
                return 0;
            }
            memcpy(key, data + key_start + 19, key_end - key_start - 19);
            //memcpy(key, "puVOuWb7rel6z2AVZBKnfw==", key_end - key_start - 19);
            key[key_end - key_start - 19] = 0;
            strcat((char*)key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            SHA1Update(&context, key, strlen((char *)key)); 
            SHA1Final(digest, &context); 

            LOG_DEBUG("%d %d %d\n", rnrn, key_start, key_end);
            //LOG_DEBUG("Sec-WebSocket-Key: %s Key Len(%d)\n", key, strlen((char*)key));
            LOG_DEBUG("Sec-WebSocket-Key: %s\n", key);
            //LOG_DEBUG("Sha1: %s\n", digest);
            static char sec_websocket_accept[128];
            base64_encode(digest, sizeof(digest), sec_websocket_accept);


            time_t t = time(NULL);
            struct tm* tmp = localtime(&t);
            static char outtime[128];
            strftime(outtime, sizeof(outtime), "Date:%a, %d %b %Y %H:%M:%S GMT\r\n", tmp);
            LOG_DEBUG("Sec-WebSocket-Accept: %s\n", sec_websocket_accept);
            send(sockfd, "HTTP/1.1 101 Switching Protocols\r\n"
                         "Connection:Upgrade\r\n"
                         "Server:lujingwei002@qq.com\r\n"
                         "Upgrade:WebSocket\r\n"
                         "Access-Control-Allow-Credentials:true\r\n"
                         "Access-Control-Allow-Headers:content-type\r\n");
            send(sockfd, outtime);
            send(sockfd, "Sec-WebSocket-Accept:");
            send(sockfd, sec_websocket_accept);
            send(sockfd, "\r\n\r\n");

            //分发到lua处理
            static char funcname[64] = "Websocket.on_session_open";
            Script::pushluafunction(funcname);
            lua_pushnumber(Script::L, session->sid);
            if (lua_pcall(Script::L, 1, 0, 0) != 0)
            {
                if (lua_isstring(Script::L, -1))
                {
                    LOG_DEBUG("Websocket.on_session_open error %s\n", lua_tostring(Script::L, -1));
                }
            }

            session->had_shake = 1;
            return rnrn + 4; 
        } else 
        {
            LOG_DEBUG("recv a frame\n");
            if(decode_one_frame(sockfd, data, datalen) == 0)
            {
                return 0;
            }
            //全部帧都到了
            return combine_all_frame(session, data, datalen);
        }
        return 0;
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
        LOG_LOG("accept a new socket sockfd(%d)\n", sockfd);
        session_init(sockfd);
        Sendbuf::create(sockfd);
        Recvbuf::create(sockfd, 1024);
        aeCreateFileEvent(Net::loop, sockfd, AE_READABLE, _ev_readable, NULL);
    }


    //发送帧
    int send_frame(int sid, int opcode, const void* data, unsigned short datalen)
    {
        LOG_DEBUG("Websocket.send_frame opcode(%d) datalen(%d)", opcode, data, datalen);
        Session* session = session_find(sid);
        if(!session)
        {
            LOG_DEBUG("websocket session not found\n");
            return 0;
        }
        int sockfd = session->sockfd;
        //统一使用2字长长度
        FrameHeader header;
        header.fin = 1;//结束帧
        header.rsv = 0;
        header.opcode = opcode;
        header.mask = 0;//没有掩码
        if (datalen >= 126)
        {
            header.payload_len = 126;
            unsigned short real_len = htons(datalen);
            _real_send(sockfd, &header, sizeof(header));
            _real_send(sockfd, &real_len, sizeof(real_len));
            _real_send(sockfd, data, datalen);
        } else 
        {
            header.payload_len = datalen;
            _real_send(sockfd, &header, sizeof(header));
            _real_send(sockfd, data, datalen);
        }
        return 0;
    }
    
    int send_binary_frame(int sid, const void* data, unsigned short datalen)
    {
        return send_frame(sid, 2, data, datalen);
    }

    //发送字符串帧
    int send_string_frame(int sid, const char* str)
    {
        return send_frame(sid, 1, str, strlen(str) + 1);
    }

   
    //底层的SEND接口
    int _real_send(int sockfd, const void* data, size_t datalen)
    {
        //插入到缓冲区
        char* buf= Sendbuf::alloc(sockfd, datalen);
        if (!buf)
        {
            return 0;
        }
        LOG_DEBUG("send %ld to sockfd(%d)\n", datalen, sockfd);
        memcpy(buf, data, datalen);
        Sendbuf::flush(sockfd, buf, datalen);
        aeCreateFileEvent(Net::loop, sockfd, AE_WRITABLE, _ev_writable, NULL);
        return datalen;
    }

    //底层的SEND接口
    int send(int sockfd, const char* data)
    {
        return _real_send(sockfd, data, strlen(data));
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
        LOG_DEBUG("websocket listen success\n");
        return 0;
    }
};
