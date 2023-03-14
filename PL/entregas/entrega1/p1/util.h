#ifndef __UTIL_H__

#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <string.h>

#define FALSO   0
#define CIERTO  1
#define TAMMSG  1024

int valida_numero(char* str);
int valida_ip(char* ip);
double randRange(double min, double max);
void log_debug(char* msg);
void check_error(int ret, char* msg);
void exit_error(char* msg);
void check_null(void* ptr, char* msg);
void check_not_null(void* ptr, char* msg);
void check_value(int ret, char* msg, int val);
void p_check_null(void* ptr, char* msg);
void p_exit_error(char* msg);
void p_check_error(int ret, char* msg);

#define __UTIL_H__
#endif
