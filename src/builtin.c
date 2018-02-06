#include <stdlib.h>
#include "builtin.h"

static struct ArgType createArgType(int argc, ClassType *args) {
  struct ArgType a;
  a.arity = argc;
  a.types = malloc(sizeof(ClassType) * argc);
  int i;
  for (i = 0; i < argc; i++) {
    a.types[i] = args[i];
  }
  return a;
}

void addBuiltinMethods() {
  struct Class *Void = getVoidClass();
  struct Method *m;
  ClassType sig[100];
  // void()
  m = addConstructor(Method_BUILTIN, Void, createArgType(0, sig));
  // void getchar()
  m = addMethod(Method_BUILTIN, Void, Void, "getchar", createArgType(0, sig));
  // void feof()
  m = addMethod(Method_BUILTIN, Void, Void, "feof", createArgType(0, sig));
  // void puts(void str)
  sig[0] = Void;
  m = addMethod(Method_BUILTIN, Void, Void, "puts", createArgType(1, sig));
  // void putchar(void bit)
  sig[0] = Void;
  m = addMethod(Method_BUILTIN, Void, Void, "putchar", createArgType(1, sig));
}
