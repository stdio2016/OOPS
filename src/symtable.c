#include <stdio.h>
#include <stdlib.h>
#include "MyHash.h"
#include "symtable.h"
#include "errReport.h"
#include "ArrayList.h"

static struct ArrayList argTypeList = {0};

void initSymTable(void) {
  ArrayList_init(&argTypeList);
}

void destroySymTable(void) {
  ArrayList_destroy(&argTypeList);
}

struct ArgType getArgumentList(void) {
  struct ArgType a;
  int i;
  a.arity = argTypeList.size;
  a.types = malloc(a.arity * sizeof(ClassType));
  for (i = 0; i < a.arity; i++) {
    a.types[i] = argTypeList.items[i];
  }
  ArrayList_clear(&argTypeList);
  return a;
}

void addParamVar(ClassType cls, const char *name) {
  ArrayList_add(&argTypeList, cls);
  printf("    param %s\n", name);
}

void addLocalVar(ClassType cls, const char *name) {
  printf("    var %s\n", name);
}
