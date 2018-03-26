#pragma once
#ifndef CODEGEN_INCLUDED
#define CODEGEN_INCLUDED
#include "ast.h"
#include "class.h"

void compileAllClasses(void);

struct Method *getBestFitMethod(struct Class *cls, const char *methodName, struct Expr *args);

#endif
