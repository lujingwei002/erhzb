#include "log/log.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


namespace Log 
{
static const char *s_level_msg[] = {"ERROR", "WARN", "LOG", "MSG", "DEBUG"};
 
static int s_log_flag = 0xffffffff;

static int s_error_counter = 0;
static int s_warn_counter = 0;

static int s_log_file = 0;
static int s_log_file_counter = 0;
static int s_log_file_max_linenum = 10000;
static char s_log_file_name[FILENAME_MAX];
static char s_log_file_dir[FILENAME_MAX];

static int log_vprint(int level, const char *fmt, va_list ap);
static void rotate_log_file(int force);


int msg(const char *fmt, ...)
{
    va_list   args;
    va_start(args, fmt);
    log_vprint(MSG, fmt, args);
    va_end(args);
    return 1;
}

int debug(const char *fmt, ...)
{
    va_list   args;
    va_start(args, fmt);
    log_vprint(DEBUG, fmt, args);
    va_end(args);
    return 1;
}

int log(const char *fmt, ...)
{
    va_list   args;
    va_start(args, fmt);
    log_vprint(LOG, fmt, args);
    va_end(args);
    return 1;
}

int warn(const char *fmt, ...)
{
    s_warn_counter++;
    va_list   args;
    va_start(args, fmt);
    log_vprint(WARN, fmt, args);
    va_end(args);
    return 1;
}

int error(const char *fmt, ...)
{
    s_error_counter++;
    va_list   args;
    va_start(args, fmt);
    log_vprint(ERROR, fmt, args);
    va_end(args);
    return 1;
}


static int log_vprint(int level, const char *fmt, va_list ap)
{
	if(!(s_log_flag & (1 << level)))
    {
		return 0;
	}
    rotate_log_file(0);
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm *tm = localtime(&t.tv_sec);
    const char *level_msg = s_level_msg[level];
    printf("[%04d-%02d-%02d %02d:%02d:%02d.%06ld][%s]", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (long)t.tv_usec, level_msg);
    vprintf(fmt, ap);
    printf("\n");
    fflush(stdout);
    return 1;
}

int log_print(int level, const char *str, int str_len)
{
	if(!(s_log_flag & (1 << level)))
    {
		return 0;
	}
    rotate_log_file(0);
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm *tm = localtime(&t.tv_sec);
    const char *level_msg = s_level_msg[level];
    printf("[%04d-%02d-%02d %02d:%02d:%02d.%06ld][%s]", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (long)t.tv_usec, level_msg);
    fwrite(str, 1, str_len, stdout);
    printf("\n");
    fflush(stdout);
    return 1;
}

static void rotate_log_file(int force)
{
    if(s_log_file != 0)
    {
        if(force || s_log_file_counter++ > s_log_file_max_linenum)
        {
            char file_path[FILENAME_MAX];
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            if(tm == NULL)
            {
                LOG_ERROR("localtime fail");
            }
            sprintf(file_path, "%s/%s_%04d_%02d_%02d_%02d_%02d_%02d", 
                s_log_file_dir,
                s_log_file_name, 
                tm->tm_year + 1900, 
                tm->tm_mon + 1, 
                tm->tm_mday, 
                tm->tm_hour, 
                tm->tm_min, 
                tm->tm_sec);
            rename(s_log_file_name, file_path);
        
            int fd = open(s_log_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            if(fd == -1)
            {
                s_log_file_counter = 0;
                LOG_ERROR("open file fail %s %s", s_log_file_name, strerror(errno));
                return;
            }
            dup2(fd, 1);
            close(fd);
            s_log_file_counter = 0;
        }
    }
}

int openlevel(int level)
{
    s_log_flag |= 1 << level;
    return 0;
}

int closeall()
{
    s_log_flag = 0;
    return 0;
}

int closelevel(int level)
{
    s_log_flag &= ~(1 << level);
    return 0;
}

int log2file(char *file_name, int file_max_linenum, char *file_dir)
{
    strcpy(s_log_file_name, file_name);
    s_log_file_max_linenum = file_max_linenum;
    strcpy(s_log_file_dir, file_dir);
    int fd = open(s_log_file_name, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if(fd == -1)
    {
        LOG_ERROR("open file fail %s %s", s_log_file_name, strerror(errno));
        return 1;
    }else
    {
        if(dup2(fd, STDOUT_FILENO) == -1)
        {
            LOG_ERROR("dup2 FAIL %s", strerror(errno));
            return 1;
        }
        close(fd);
        s_log_file = 1; 
        return 0;
    }
}

};
