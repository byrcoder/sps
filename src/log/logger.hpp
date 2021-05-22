#ifndef SPS_LOGGER_HPP
#define SPS_LOGGER_HPP

#include <stdio.h>

#define sp_error(msg, ...) printf(msg, ##__VA_ARGS__)
#define sp_trace(msg, ...) printf(msg, ##__VA_ARGS__)
#define sp_info(msg, ...)  printf(msg, ##__VA_ARGS__)

#endif // SPS_LOGGER_HPP
