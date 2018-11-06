#ifndef __LOGGER_H__
#define __LOGGER_H__

#define LOG(fmt, ...) fprintf(stdout, "[DEBUG] %s:\n%s:%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif