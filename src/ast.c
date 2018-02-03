#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "class.h"

char *dupstr(const char *str) {
  size_t n = strlen(str);
  char *nstr = malloc(n + 1);
  strcpy(nstr, str);
  return nstr;
}

void destroyArgType(struct ArgType a) {
  free(a.types);
}

struct Constant copyConst(struct Constant c) {
  struct Constant d = c;
  if (c.type == Type_STRING) {
    d.str = dupstr(c.str);
  }
  return d;
}

void destroyConst(struct Constant c) {
  if (c.type == Type_STRING) {
    free(c.str);
  }
}

void showConst(struct Constant c) {
  int n;
  switch (c.type) {
    case Type_STRING: n += printf("\"%s\"", c.str); break;
  }
}

struct Expr *createExpr(enum Operator op, struct Expr *arg1, struct Expr *arg2) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = op;
  n->type = NULL;
  n->next = NULL;
  n->args = arg1;
  if (arg1 != NULL) {
    arg1->next = arg2;
  }
  return n;
}

struct Expr *createLitExpr(struct Constant lit) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_LIT;
  n->type = getVoidClass();
  n->next = NULL;
  n->lit = lit;
  return n;
}

struct Expr *createVarExpr(char *name) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_VAR;
  n->type = NULL;
  n->next = NULL;
  n->name = name;
  return n;
}

struct Expr *createLocalVarExpr(int id) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_LOCAL;
  n->type = NULL;
  n->next = NULL;
  n->varId = id;
  return n;
}

struct Expr *createFuncExpr(struct Expr *name, struct Expr *args) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_FUNC;
  n->type = NULL;
  n->next = NULL;
  n->args = name;
  n->args->next = args;
  return n;
}

void destroyExpr(struct Expr *expr) {
  struct Expr *p = expr, *q;
  while (p != NULL) {
    if (p->op == Op_VAR) {
      free(p->name);
    }
    else if (p->op == Op_LIT) {
      destroyConst(p->lit);
    }
    else {
      destroyExpr(p->args);
    }
    q = p;
    p = p->next;
    free(q);
  }
}

char *OpName[] = {
  "",
  "=", // a = b, requires type annotation
  "NEW", // new a(args)
  "DOT", // obj.var or obj.method
  "CALL", // f(x)
  "<literal> ", // literal
  "<this>", // this
  "<super>", // super
  "<var> ", // variable reference or class or method name
  "<local> " // local variable reference
};

void showExprType(struct Expr *expr) {
  if (expr->type != NULL) {
    printf(": %s", expr->type->name);
  }
}

void showExpr(struct Expr *expr, int depth) {
  int i;
  for (i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("%s", OpName[expr->op]);
  if (expr->op == Op_VAR) {
    printf("%s", expr->name);
    showExprType(expr);
    puts("");
  }
  else if (expr->op == Op_LOCAL) {
    printf("%d", expr->varId);
    showExprType(expr);
    puts("");
  }
  else if (expr->op == Op_LIT) {
    showConst(expr->lit);
    showExprType(expr);
    puts("");
  }
  else {
    showExprType(expr);
    puts("");
    struct Expr *p = expr->args;
    while (p != NULL) {
      showExpr(p, depth + 1);
      p = p->next;
    }
  }
}

void initExprList(struct ExprList *list) {
  list->first = list->last = NULL;
}

void addToExprList(struct ExprList *list, struct Expr *expr) {
  if (list->first == NULL) {
    list->first = expr;
  }
  else {
    list->last->next = expr;
  }
  list->last = expr;
}
