#ifndef _RECVBUF_H_
#define _RECVBUF_H_
/*
 *  Recvbuf.create(sockfd, size)
 *  Recvbuf.free(sockfd)
*/

namespace  Recvbuf
{
    int create(int sockfd, int size);
    int free(int sockfd);
    int buf2line(int sockfd);
    int wskip(int sockfd, int len);
    int rskip(int sockfd, int len);
    char* getwptr(int sockfd);
    char* getrptr(int sockfd);
    int find(int sockfd, const char *str, int startpos); 
    int datalen(int sockfd);
    int buflen(int sockfd);
    int bufremain(int sockfd);

    unsigned char readint8(int sockfd);
    short readint16(int sockfd);
    int readint32(int sockfd);
    unsigned short getuint16(int sockfd);
    unsigned int getuint32(int sockfd);
    unsigned char getint8(int sockfd);
    short getint16(int sockfd);
    int getint32(int sockfd);
};


#endif
