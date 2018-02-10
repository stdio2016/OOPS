#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "errReport.h"

void compileAllClasses(void) {
  int i;
  for (i = 0; i < classCount; i++) {
    struct Class *c = classTable[i];
    if (c == getVoidClass()) continue;
    size_t nlen = strlen(c->name);
    genClassHeader(c);
    int j;
    for (j = 0; j < c->methodCount; j++) {
      struct Method *m = c->methodTable[j];
      if (m->thisClass == c) { // own class
        genMethod(m);
      }
    }
  }
}

void genClassHeader(struct Class *c) {

}

void genMethod(struct Method *m) {
  printf("%s %s::%s", m->returnType->name, m->thisClass->name, m->name);
  showSignature(m->args);
  puts(" compiling");
  if (m->ast != NULL) {
    struct ClassTypeAndIntPair status = genStatement(m->ast, 1, m->thisClass);
    if (status.n == 0) { // no statement
      printf("  this\n  return\n");
    }
    else { // has statement
      if (status.type == NULL) puts("  (type unknown)");
      else printf("  (return type %s)\n", status.type->name);
      printf("  return\n");
    }
  }
}

struct ClassTypeAndIntPair genStatement(struct Statement *s, int first, ClassType thisType) {
  struct ClassTypeAndIntPair a;
  int ret = 0;
  a.type = NULL;
  while (s != NULL && !ret) {
    if (s->type == Stmt_COMPOUND) {
      struct ClassTypeAndIntPair st = genStatement(s->stmt, first, thisType);
      if (st.n == -1) {
        ret = 1;
        a.type = st.type;
      }
      else if (st.n == 1) {
        first = 0;
        a.type = st.type;
      }
    }
    else if (s->type == Stmt_RETURN || s->type == Stmt_SIMPLE) {
      if (!first) {
        printf("  pop\n");
      }
      genExpr(s->expr, thisType);
      a.type = s->expr->type;
      if (s->type == Stmt_RETURN) {
        ret = 1;
      }
      first = 0;
    }
    else {
      printf("unknown statement type %d\n", s->type);
      exit(EXIT_FAILURE);
    }
    s = s->next;
  }
  if (ret && s != NULL) {
    //semanticError("unreachable statement\n");
    printf("warning: unreachable statement\n");
  }
  a.n = ret ? -1 : (first ? 0 : 1);
  return a;
}

void genExpr(struct Expr *expr, ClassType thisType) {
  switch (expr->op) {
    case Op_ASSIGN: genAssign(expr, thisType); break;
    case Op_NEW: break;
    case Op_DOT: genDotExpr(expr, thisType); break;
    case Op_FUNC: break;
    case Op_LIT: printf("  str \"%s\"\n", expr->lit.str); break;
    case Op_THIS: printf("  this\n"); expr->type = thisType; break;
    case Op_SUPER: printf("  this\n"); expr->type = thisType; break;
    case Op_VAR: genVarExpr(expr, thisType); break;
    case Op_LOCAL: printf("  load %d\n", expr->varId); break;
    case Op_NULL: printf("  null\n"); break;
    default:
      printf("unknown expression type %d\n", expr->op);
      exit(EXIT_FAILURE);
  }
}

void genTypeConvert(struct Class *from, struct Class *to) {
  if (from == NULL) {
    return;
  }
  if (isKindOf(from, to)) return; // no need to convert
  if (isKindOf(to, from)) {
    printf("  convert %d (class %s)\n", to->id, to->name);
  }
  else {
    semanticError("cannot convert %s to %s\n", from->name, to->name);
  }
}

void genAssign(struct Expr *expr, ClassType thisType) {
  genExpr(expr->args->next, thisType);
  /*
  * assign can be:
  * 1. obj DOT name = expr
  * 2. LOCAL = expr
  * 3. VAR = expr
  */
  if (expr->args->op == Op_DOT) {
    struct Class *c;
    if (expr->args->args->op == Op_THIS) c = thisType;
    else if (expr->args->args->op == Op_SUPER) c = thisType->base;
    else {
      semanticError("only field variable of this or super is accessible\n");
      return ;
    }
    char *name = expr->args->args->next->name;
    struct Field *f = MyHash_get(&c->fields, name);
    if (f == NULL) {
      semanticError("undefined variable %s%s\n", name, c == thisType ? "" : " in super class");
    }
    else {
      expr->type = f->type;
      genTypeConvert(expr->args->next->type, expr->type);
      printf("  putfield %d\n", f->id);
    }
  }
  else if (expr->args->op == Op_LOCAL) {
    expr->type = expr->args->type;
    genTypeConvert(expr->args->next->type, expr->type);
    printf("  store %d\n", expr->args->varId);
  }
  else if (expr->args->op == Op_VAR) {
    struct Field *f = MyHash_get(&thisType->fields, expr->args->name);
    if (f == NULL) {
      semanticError("undefined variable: %s\n", expr->args->name);
    }
    else {
      expr->type = f->type;
      genTypeConvert(expr->args->next->type, expr->type);
      printf("  putfield %d\n", f->id);
    }
  }
  else {
    semanticError("left hand side of \"=\" is not an lvalue\n");
  }
}

void genVarExpr(struct Expr *expr, ClassType thisType) {
  // can only be field variable
  struct Field *f = MyHash_get(&thisType->fields, expr->name);
  if (f == NULL) {
    semanticError("undefined variable: %s\n", expr->name);
  }
  else {
    expr->type = f->type;
    printf("  getfield %d\n", f->id);
  }
}

void genDotExpr(struct Expr *expr, ClassType thisType) {
  // can only be field variable
  struct Class *c;
  if (expr->args->op == Op_THIS) c = thisType;
  else if (expr->args->op == Op_SUPER) c = thisType->base;
  else {
    semanticError("only field variable of this or super is accessible\n");
    return ;
  }
  char *name = expr->args->next->name;
  struct Field *f = MyHash_get(&c->fields, name);
  if (f == NULL) {
    semanticError("undefined variable %s%s\n", name, c == thisType ? "" : " in super class");
  }
  else {
    expr->type = f->type;
    genTypeConvert(expr->args->next->type, expr->type);
    printf("  getfield %d\n", f->id);
  }
}
