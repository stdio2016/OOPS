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
    case Op_ASSIGN: break;
    case Op_NEW: break;
    case Op_DOT: break;
    case Op_FUNC: break;
    case Op_LIT: printf("  str \"%s\"\n", expr->lit.str); break;
    case Op_THIS: printf("  this\n"); expr->type = thisType; break;
    case Op_SUPER: printf("  this\n"); expr->type = thisType; break;
    case Op_VAR: break;
    case Op_LOCAL: printf("  load %d\n", expr->varId); break;
    case Op_NULL: printf("  null\n"); break;
    default:
      printf("unknown expression type %d\n", expr->op);
      exit(EXIT_FAILURE);
  }
}
