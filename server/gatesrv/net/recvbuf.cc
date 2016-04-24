#include "recvbuf.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

namespace Recvbuf 
{ 

    typedef struct RBuf{
        int buf_len;
        int rptr;
        int wptr;
        char *buf;
    }RBuf;

    //1M * 4 * 4 = 16M
#define MAX_SOCKET 1048576
    static RBuf rbufs[MAX_SOCKET];
    static int mem_total = 0;
#define fd2rbuf(sockfd)  (&Recvbuf::rbufs[sockfd])

    static int get_buf(int sockfd, void *vbuf, int buf_len){
        RBuf *self = fd2rbuf(sockfd);
        char *buf = self->buf + self->rptr;
        memcpy(vbuf, buf, buf_len);
        return buf_len;
    }

    static int read_buf(int sockfd, void *vbuf, int buf_len){
        RBuf *self = fd2rbuf(sockfd);
        char *buf = self->buf + self->rptr;
        memcpy(vbuf, buf, buf_len);
        self->rptr += buf_len;
        return buf_len;
    }

    int free(int sockfd) {
        return 0;
    }


    int bufremain(int sockfd){
        RBuf *self = fd2rbuf(sockfd);
        return self->buf_len - self->wptr;
    }

    int buflen(int sockfd){
        RBuf *self = fd2rbuf(sockfd);
        return self->buf_len;
    }

    int datalen(int sockfd){
        RBuf *self = fd2rbuf(sockfd);
        int datalen = self->wptr - self->rptr;
        return datalen;
    }

    int find(int sockfd, const char *str, int startpos) 
    {
        size_t str_len;
        int i;
        str_len = strlen(str);
        RBuf *self = fd2rbuf(sockfd);
        if (self->rptr + (int)str_len >= self->wptr) 
        {
            //  LOG("rptr(%d) str_len(%d) wptr(%d)", self->rptr, str_len, self->wptr);
            return -1;
        }
        //LOG("i(%d) wptr(%d)", self->rptr + startpos, self->wptr);
        for (i = self->rptr + startpos; i < self->wptr; i++) 
        {
            if (!strncmp(self->buf + i, str, str_len)) 
            {
                return i - self->rptr;
            }
            // printf("ee %d c(%c)", i, *(self->buf + i));
        }
        return -1;
    }

    char* getrptr(int sockfd)
    {
        RBuf *self = fd2rbuf(sockfd);
        char *ptr = self->buf + self->rptr;
        return ptr;
    }


    char* getwptr(int sockfd)
    {
        RBuf *self = fd2rbuf(sockfd);
        char *ptr = self->buf + self->wptr;
        return ptr;
    }

    int rskip(int sockfd, int len)
    {
        RBuf *self = fd2rbuf(sockfd);
        if (self->wptr - self->rptr < len) 
        {
            return 0;
        }
        self->rptr += len;
        return len;
    }

    int wskip(int sockfd, int len)
    {
        RBuf *self = fd2rbuf(sockfd);
        self->wptr += len;
        return len;
    }

    int buf2line(int sockfd)
    {
        RBuf *self = fd2rbuf(sockfd);
        if(self->rptr == 0) {
            return 1;
        }
        int len = self->wptr - self->rptr;
        memmove(self->buf, self->buf + self->rptr, len);
        self->rptr = 0;
        self->wptr = len;
        return 0;
    }

    unsigned char readint8(int sockfd)
    {
        unsigned char val = 0;
        read_buf(sockfd, (char *)&val, sizeof(unsigned char));
        return val;
    }

    short readint16(int sockfd)
    {
        short val = 0;
        read_buf(sockfd, (char *)&val, sizeof(short));
        return val;
    }

    int readint32(int sockfd)
    {
        int val = 0;
        read_buf(sockfd, (char *)&val, sizeof(int));
        return val;
    }

    unsigned char getint8(int sockfd)
    {
        unsigned char val = 0;
        get_buf(sockfd, (char *)&val, sizeof(unsigned char));
        return val;
    }

    unsigned int getuint32(int sockfd)
    {
        unsigned int val = 0;
        get_buf(sockfd, (char *)&val, sizeof(unsigned int));
        return val;
    }

    unsigned short getuint16(int sockfd)
    {
        unsigned short val = 0;
        get_buf(sockfd, (char *)&val, sizeof(unsigned short));
        return val;
    }

    short getint16(int sockfd)
    {
        short val = 0;
        get_buf(sockfd, (char *)&val, sizeof(short));
        return val;
    }

    int getint32(int sockfd)
    {
        int val = 0;
        get_buf(sockfd, (char *)&val, sizeof(int));
        return val;
    }

    int create(int sockfd, int size)
    {
        RBuf *self = fd2rbuf(sockfd);

        if(self->buf_len < size)
        {
            if(self->buf != NULL)
            {
                ::free(self->buf);
                self->buf_len = 0;
                self->buf = NULL;
                mem_total -= self->buf_len;
            }
            self->buf = (char *)malloc(size);
            if(self->buf == NULL)
            {
                printf("malloc fail");
                return 1;
            }
            self->buf_len = size;
            mem_total += size;
        }
        self->rptr = 0;
        self->wptr = 0;
        printf("recvbuf create sockfd(%d) buf_len(%d)\n", sockfd, self->buf_len);
        return 0;
    }

};
