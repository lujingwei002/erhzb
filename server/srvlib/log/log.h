
#ifndef _LOG_H_
#define _LOG_H_

const int ERROR =  0;
const int WARN  =  1;
const int LOG   =  2;
const int MSG   =  3;
const int DEBUG =  4;

namespace Log 
{

int log_print(int level, const char *str, int str_len);
int log(const char *fmt, ...);
int warn(const char *fmt, ...);
int error(const char *fmt, ...);
int msg(const char *fmt, ...);
int debug(const char *fmt, ...);

int openlevel(int level);
int closelevel(int level);
int closeall();

int log2file(char *file_name, int file_max_linenum, char *file_dir);

};

#define LOG_ERROR(str, ...)     Log::error("%s:%d", __FILE__, __LINE__);\
                                Log::error(str, ## __VA_ARGS__);

#define LOG_WARN(str, ...)      Log::warn("%s:%d", __FILE__, __LINE__);\
                                Log::warn(str, ## __VA_ARGS__);

#define LOG_LOG(str, ...)       Log::log("%s:%d", __FILE__, __LINE__);\
                                Log::log(str, ## __VA_ARGS__);

#define LOG_MSG(str, ...)       Log::msg(str, ## __VA_ARGS__);

#define LOG_DEBUG(str, ...)     Log::debug("%s:%d", __FILE__, __LINE__);\
                                Log::debug(str, ## __VA_ARGS__);

#endif
