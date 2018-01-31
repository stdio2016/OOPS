#include <stdio.h>
#include <stdlib.h>
#include "class.h"

ClassType getVoidClass() {
  return NULL;
}

ClassType getClass(const char *name) {
  return NULL;
}

ClassType createClass(const char *name, ClassType baseClass) {
  printf("class %s\n", name);
  return NULL;
}

void addField(ClassType cls, ClassType type, const char *name) {
  printf("  field %s\n", name);
}

void showClassSignature(struct ArgType args) {
  int i;
  for (i = 0; i < args.arity; i++) {
    if (i > 0) putchar(',');
    if (args.types[i] == NULL) {
      printf("??");
    }
    else {
      printf("%s,", args.types[i]->name);
    }
  }
}

void addMethod(ClassType cls, ClassType returnType, const char *name, struct ArgType arguments) {
  printf("  method %s(", name);
  showClassSignature(arguments);
  puts(")");
  /* TODO add method */
}

void addConstructor(ClassType cls, const char *name, struct ArgType arguments) {
  printf("  constructor %s(", name);
  showClassSignature(arguments);
  puts(")");
  /* TODO add method */
}
