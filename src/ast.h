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

// remember to change OpName in ast.c after adding operator in enum
enum Operator {
  Op_NONE,
  Op_ASSIGN, // a = b, requires type annotation
  Op_NEW, // new a(args)
  Op_DOT, // obj.var or obj.method
  Op_FUNC, // f(x)
  Op_LIT, // literal
  Op_THIS, // this
  Op_SUPER, // super
  Op_VAR, // variable reference or class or method name
  Op_LOCAL // local variable reference
};

enum TypeEnum {
  Type_STRING, Type_VOID
};

struct Constant {
  enum TypeEnum type;
  union {
    char *str;
  };
};

struct ExprList {
  struct Expr *first;
  struct Expr *last;
};

struct Expr {
  enum Operator op;
  ClassType type;
  int lineno;
  union {
    struct Constant lit;
    char *name; // variable/class/method name
    int varId; // local variable id
    struct Expr *args;
  };
  struct Expr *next;
};

char *dupstr(const char *str);

void destroyArgType(struct ArgType a);

struct Constant copyConst(struct Constant c);
void destroyConst(struct Constant c);
void showConst(struct Constant c);

struct Expr *createExpr(enum Operator op, struct Expr *op1, struct Expr *op2);
struct Expr *createLitExpr(struct Constant lit);
struct Expr *createVarExpr(char *name);
struct Expr *createLocalVarExpr(int id);
struct Expr *createFuncExpr(struct Expr *name, struct Expr *args);
void destroyExpr(struct Expr *expr);
void showExpr(struct Expr *expr, int depth);

void initExprList(struct ExprList *list);
void addToExprList(struct ExprList *list, struct Expr *expr);

#endif
