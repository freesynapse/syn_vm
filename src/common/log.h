#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <errno.h>

//
#define LOG_INFO(msg, ...) do { fprintf(stdout, "[INFO] %s(): " msg "", __func__, ##__VA_ARGS__); } while (0)
#define LOG_WARNING(msg, ...) do { fprintf(stderr, "[WARNING] %s(): " msg "", __func__, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(msg, ...) do { fprintf(stderr, "[ERROR] %s(): " msg "", __func__, ##__VA_ARGS__); exit(1); } while(0)


#endif // __LOG_H
