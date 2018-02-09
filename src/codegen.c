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
    int status = genStatement(m->ast, 1);
    if (status == 0) printf("  this\n  return\n");
    else if (status == 1) printf("  return\n");
  }
}

int genStatement(struct Statement *s, int first) {
  int ret = 0;
  while (s != NULL && !ret) {
    if (s->type == Stmt_COMPOUND) {
      int st = genStatement(s->stmt, first);
      if (st == -1) ret = 1;
      else if (st == 1) first = 0;
    }
    else if (s->type == Stmt_RETURN || s->type == Stmt_SIMPLE) {
      if (!first) {
        printf("  pop\n");
      }
      showExpr(s->expr, 1);
      if (s->type == Stmt_RETURN) {
        ret = 1;
        printf("  return\n");
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
  return ret ? -1 : (first ? 0 : 1);
}
