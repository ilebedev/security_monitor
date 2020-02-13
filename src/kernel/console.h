#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

void print_char(char c);
void print_str(char* s);
void print_int(uint64_t n);

void console_putchar(char c);
uint64_t console_getchar();

void printm(const char* s, ...);
void vprintm(const char *s, va_list args);
void putstring(const char* s);

#endif // CONSOLE_H
