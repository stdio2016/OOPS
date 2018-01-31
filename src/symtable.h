#pragma once
#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include "ast.h"
#include "class.h"

void initSymTable(void);
void destroySymTable(void);

struct ArgType getArgumentList(void);

void addParamVar(ClassType cls, const char *name);
void addLocalVar(ClassType cls, const char *name);

#endif
