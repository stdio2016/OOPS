#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "class.h"

extern int linenum; // defined in oops.lex

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
    d.strId = c.strId;
  }
  return d;
}

void destroyConst(struct Constant c) {
  if (c.type == Type_STRING) {
    //free(c.str);
  }
}

void showConst(struct Constant c) {
  int n;
  switch (c.type) {
    case Type_STRING: n += printf("%d", c.strId); break;
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
  n->linenum = linenum;
  return n;
}

struct Expr *createLitExpr(struct Constant lit) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_LIT;
  n->type = getVoidClass();
  n->next = NULL;
  n->lit = lit;
  n->linenum = linenum;
  return n;
}

struct Expr *createVarExpr(char *name) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_VAR;
  n->type = NULL;
  n->next = NULL;
  n->name = name;
  n->linenum = linenum;
  return n;
}

struct Expr *createLocalVarExpr(int id) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_LOCAL;
  n->type = NULL;
  n->next = NULL;
  n->varId = id;
  n->linenum = linenum;
  return n;
}

struct Expr *createFuncExpr(struct Expr *name, struct Expr *args) {
  struct Expr *n = malloc(sizeof(struct Expr));
  n->op = Op_FUNC;
  n->type = NULL;
  n->next = NULL;
  n->args = name;
  n->args->next = args;
  n->linenum = linenum;
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
    else if (p->op == Op_LOCAL) {
      // do nothing
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
  "<local> ", // local variable reference
  "<null>"
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

struct Statement *createStmt(enum StatementType type, struct Expr *expr) {
  struct Statement *s = malloc(sizeof(*s));
  s->type = type;
  s->expr = expr;
  s->next = NULL;
  return s;
}

struct Statement *createCompoundStmt(struct StatementList body) {
  struct Statement *s = malloc(sizeof(*s));
  s->type = Stmt_COMPOUND;
  s->stmt = body.first;
  s->next = NULL;
  return s;
}

void destroyStmt(struct Statement *stmt) {
  struct Statement *p = stmt, *q;
  while (p != NULL) {
    if (p->type == Stmt_COMPOUND) {
      destroyStmt(p->stmt);
    }
    else if (p->type == Stmt_SIMPLE || p->type == Stmt_RETURN) {
      destroyExpr(p->expr);
    }
    else {
      printf("Unknown statement type %d", p->type);
    }
    q = p;
    p = p->next;
    free(q);
  }
}

void initStmtList(struct StatementList *list) {
  list->first = list->last = NULL;
}

void addToStmtList(struct StatementList *list, struct Statement *stmt) {
  if (stmt == NULL) return ;
  if (list->first == NULL) {
    list->first = stmt;
  }
  else {
    list->last->next = stmt;
  }
  list->last = stmt;
}
