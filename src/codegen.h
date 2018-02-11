#pragma once
#ifndef CODEGEN_INCLUDED
#define CODEGEN_INCLUDED
#include "ast.h"
#include "class.h"

void compileAllClasses(void);

void genClassHeader(struct Class *c);
void genMethod(struct Method *m);

struct ClassTypeAndIntPair{
  ClassType type;
  int n;
};
// return type of the last statement
// return 0: no value on stack
// return 1: a value on stack
// return -1: value returned
struct ClassTypeAndIntPair genStatement(struct Statement *s, int first, ClassType thisType);

void genExpr(struct Expr *expr, ClassType thisType);
void genTypeConvert(struct Class *from, struct Class *to);
void genAssign(struct Expr *expr, ClassType thisType);
void genVarExpr(struct Expr *expr, ClassType thisType);
void genDotExpr(struct Expr *expr, ClassType thisType);
void genNewExpr(struct Expr *expr, ClassType thisType);
void genFuncExpr(struct Expr *expr, ClassType thisType);

int getBestFitMethodId(struct Class *cls, const char *methodName, struct Expr *args);

#endif
