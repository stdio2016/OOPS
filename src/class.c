#include <stdio.h>
#include <stdlib.h>
#include "class.h"

struct MyHash allClasses;
ClassType VoidClass;

void initClassTable() {
  MyHash_init(&allClasses, MyHash_strcmp, MyHash_strhash);
  VoidClass = createClass("void", NULL);
}

void classTableDestructor(struct HashBucket *hb) {
  destroyClass(hb->value);
  free(hb);
}

void destroyClassTable() {
  MyHash_destroy(&allClasses, classTableDestructor);
}

ClassType getVoidClass() {
  return VoidClass;
}

ClassType getClass(const char *name) {
  ClassType t = MyHash_get(&allClasses, name);
  if (t != NULL) return t;
  t = createClass(name, NULL);
  return t;
}

ClassType createClass(const char *name, ClassType baseClass) {
  ClassType t = MyHash_get(&allClasses, name);
  if (t != NULL) {
    return t;
  }
  t = malloc(sizeof(struct Class));
  t->name = dupstr(name);
  t->base = baseClass;
  MyHash_set(&allClasses, t->name, t);
  return t;
}

void destroyClass(ClassType cls) {
  free(cls->name);
  free(cls);
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
      printf("%s", args.types[i]->name);
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

int showClassName(ClassType type) {
  return printf("%s", type->name);
}
