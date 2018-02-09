#pragma once
#ifndef CODEGEN_INCLUDED
#define CODEGEN_INCLUDED
#include "ast.h"
#include "class.h"

void compileAllClasses(void);

void genClassHeader(struct Class *c);
void genMethod(struct Method *m);

// return 0: no value on stack
// return 1: a value on stack
// return -1: value returned
int genStatement(struct Statement *s, int first);

#endif
