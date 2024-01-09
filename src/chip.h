/*
	shared header file
*/
#ifndef CHIP_H
#define CHIP_H

#include <stdbool.h>
#include <stddef.h>

#define COLOR_BLACK "\033[1;30m"
#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_WHITE "\033[1;37m"
#define COLOR_RESET "\033[0m"

char             *strdup(const char *s);
char             *read_file(char *file);

#endif