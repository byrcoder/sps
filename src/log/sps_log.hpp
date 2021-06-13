#ifndef SPS_LOG_HPP
#define SPS_LOG_HPP

#include <stdio.h>

#include <sps_auto_header.hpp>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define sp_error(msg , ...) printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_warn(msg , ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_trace(msg, ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_info(msg, ...)   printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)

#ifdef SPS_HEAVY_LOG
#define sp_debug(msg, ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#else
#define sp_debug(msg, ...)
#endif

#endif  // SPS_LOG_HPP
