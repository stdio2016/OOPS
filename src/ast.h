#pragma once
#ifndef AST_INCLUDED
#define AST_INCLUDED
#include <stdbool.h>

struct Class;

struct ArgType {
  int arity;
  struct Class **types;
};

typedef struct Class *ClassType;

char *dupstr(const char *str);

void destroyArgType(struct ArgType a);

#endif
