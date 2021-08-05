#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h> 

#define LOG_FILE stdout
#define LOG_ERROR(...) fprintf(LOG_FILE,"\033[31m[ERROR]\033[0m " __VA_ARGS__);
#define LOG_WARNING(...) fprintf(LOG_FILE,"\033[33m[WARNING]\033[0m " __VA_ARGS__);
#define LOG_SUCCESS(...) fprintf(LOG_FILE,"\033[32m[SUCCESS]\033[0m " __VA_ARGS__);
#define LOG(...) fprintf(LOG_FILE, __VA_ARGS__);

#endif // LOGGER_H