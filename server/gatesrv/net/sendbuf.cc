#include "sendbuf.h"

#include <stdio.h>
#include <stdlib.h>
 
namespace Sendbuf {

typedef struct BBuf_Block{
	struct BBuf_Block *next;
	int buf_size;  //buf的大小 
    int buf_len;   //buf已经使用的大小 
	int rptr;      //读指针
    int unuse;     //字节对齐
	char buf[0];
}BBuf_Block;

typedef struct BBuf{
	BBuf_Block *block_head;
	BBuf_Block *block_tail;
}BBuf;

#define MAX_SOCKET 65536
#define MAX_TABLE 32
static BBuf bbufs[MAX_SOCKET];
static BBuf_Block *block_table[MAX_TABLE];
#define fd2bbuf(sockfd) (&bbufs[sockfd])

static int hash(unsigned int size)
{
	unsigned int index = 0;
	while(((unsigned int)(1<<index)) < size)
    {
        index++;
	}
	return index;
}

static void bbuf_free_block(BBuf_Block *block)
{
    //printf("free block :%d/%d\n", block->buf_len, block->buf_size);
	int idx = hash(block->buf_size + sizeof(BBuf_Block));
	if(block_table[idx] == 0)
    {
		block->next = 0;
		block_table[idx] = block;
	}else
    {
		block->next = block_table[idx];
		block_table[idx] = block;
	}
}

int create(int sockfd)
{
	bbufs[sockfd].block_head = 0;
	bbufs[sockfd].block_tail = 0;
    return 0;
}

int free(int sockfd)
{
	BBuf *self = fd2bbuf(sockfd);
    BBuf_Block *block = self->block_head;
    while(block != 0)
    {
        BBuf_Block* next = block->next;
        bbuf_free_block(block);
        block = next;
    }
    return 0;
}

char* get_read_ptr(int sockfd)
{
	BBuf *self = fd2bbuf(sockfd);
	if(self->block_head == 0)
    {
		return 0;
	}
	char *ptr = self->block_head->buf + self->block_head->rptr;
    return ptr;
}

int datalen(int sockfd)
{
	BBuf *self = fd2bbuf(sockfd);
	if(!self->block_head)
    {
		return 0;
	}
	int len = self->block_head->buf_len - self->block_head->rptr;
    return len;
}


int skip_read_ptr(int sockfd, int len)
{
	BBuf *self = fd2bbuf(sockfd);
    if(self->block_head == 0)
    {
        return 1;
    }
	self->block_head->rptr += len;
	if(self->block_head->rptr >= self->block_head->buf_len)
    {
		BBuf_Block *block = self->block_head;
		self->block_head = self->block_head->next;
		if(self->block_head == 0)
        {
			self->block_tail = 0;
		}
		bbuf_free_block(block);
	}
	return 0;
}

int flush(int sockfd, char *buf, int buf_len)
{
	BBuf *self = fd2bbuf(sockfd);
    
    if(self->block_tail == 0){
        printf("tail is null\n");
        return 1;
    }
    if(self->block_tail->buf_size - self->block_tail->buf_len < buf_len)
    {
        printf("size is not enougth %d/%d\n", self->block_tail->buf_len, self->block_tail->buf_size);
        return 1;
    }
    self->block_tail->buf_len += buf_len;
    return 0;
}


char * alloc(int sockfd, int need_size)
{
	BBuf *self = fd2bbuf(sockfd);
    BBuf_Block *block = self->block_tail;
    if(block != 0&& block->buf_size - block->buf_len >= need_size)
    {
        char *ptr = block->buf + block->buf_len;
        return ptr;
    }
    //申请一块新的
	int real_size = need_size + sizeof(BBuf_Block);
    //这里加上策略 TODO, buf的大小决定了, 系统调用write的次数和申请的次数
    if(real_size <= 4096)
    {
        real_size = 4096;
    }
	int idx = hash(real_size);
	if(block_table[idx] == 0)
    {
        int malloc_size = 1<<idx;
        //printf("malloc size: %d/%d\n", need_size, malloc_size - sizeof(BBuf_Block));
		BBuf_Block *block = (BBuf_Block *)malloc(malloc_size);
		if(block == 0)
        {
            printf("malloc fail\n");
            return 0;
		}
		block_table[idx] = block;
		block->next = 0;
		block->buf_size = malloc_size - sizeof(BBuf_Block);
	}
    //申请失败
    if(block_table[idx] == 0)
    {
        printf("malloc fail\n");
        return 0;
    }
	block = block_table[idx];
	block_table[idx] = block_table[idx]->next;

	block->next = 0;
	block->rptr = 0;
    block->buf_len = 0;

    if(self->block_tail == 0)
    {
        self->block_tail = self->block_head = block;
        block->next = 0;
    }else
    {
        self->block_tail->next = block;
        self->block_tail = block;
        block->next = 0;
    }
    return block->buf;
}

};

