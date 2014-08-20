#ifndef __NATIVE_LOGGING__
#define __NATIVE_LOGGING__

#include <cstdio>

#define LOG_INFO(x,...) do {\
    printf("INFO (%s) : ", x);\
    printf(__VA_ARGS__);\
    printf("\n");\
} while(0)

#define LOG_ERROR(x,...) do {\
    printf("ERROR (%s) : ", x);\
    printf(__VA_ARGS__);\
    printf("\n");\
} while(0)

#endif
