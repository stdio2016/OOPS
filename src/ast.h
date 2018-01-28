#pragma once
#ifndef AST_INCLUDED
#define AST_INCLUDED
#include <stdbool.h>

struct Class;

typedef struct Class *ClassType;

char *dupstr(const char *str);

#endif
