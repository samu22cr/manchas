#include <stdio.h>
#include <stdarg.h>
#include "log.h"

void log_warning(char *fmt, ...) {
	va_list var_args;
	va_start(var_args, fmt);
	printf("\033[33m[WARNING]: \033[0m");
	vprintf(fmt, var_args);
	va_end(var_args);
	printf("\n");
}

void log_error(char *fmt, ...) {
	va_list var_args;
	va_start(var_args, fmt);
	fprintf(stderr, "\033[0;31m[ERROR]: \033[0m"); 
	vfprintf(stderr, fmt, var_args);
	va_end(var_args);
	fprintf(stderr, "\n"); 
}

void log_info(char *fmt, ...) {
	va_list var_args;
	va_start(var_args, fmt);
	printf("\033[0;32m[INFO]: \033[0m"); 
	vprintf(fmt, var_args);
	va_end(var_args);
	printf("\n");
}


void log_debug(char *fmt, ...) {
	va_list var_args;
	va_start(var_args, fmt);
	printf("\033[0;34m[DEBUG]: \033[0m"); 
	vprintf(fmt, var_args);
	va_end(var_args);
	printf("\n");
}

