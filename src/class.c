#include <stdio.h>
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

void addMethod(ClassType cls, ClassType returnType, const char *name, struct ArgType arguments) {
  printf("  method %s\n", name);
}

void addConstructor(ClassType cls, const char *name, struct ArgType arguments) {
  printf("  constructor %s\n", name);
}
