#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h> 

#define LOG_FILE stdout
#define LOG_ERROR(msg, vars...) fprintf(LOG_FILE,"\033[31m[ERROR]\033[0m "msg,##vars);
#define LOG_WARNING(msg, vars...) fprintf(LOG_FILE,"\033[33m[WARNING]\033[0m "msg,##vars);
#define LOG_SUCCESS(msg, vars...) fprintf(LOG_FILE,"\033[32m[SUCCESS]\033[0m "msg,##vars);
#define LOG(msg, vars...) fprintf(LOG_FILE,msg,##vars);

#endif // LOGGER_H