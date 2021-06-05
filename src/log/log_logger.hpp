#ifndef SPS_LOG_LOGGER_HPP
#define SPS_LOG_LOGGER_HPP

#include <stdio.h>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define sp_error(msg , ...) printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_trace(msg, ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_info(msg, ...)   printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)

#endif // SPS_LOG_LOGGER_HPP
