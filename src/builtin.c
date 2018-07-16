#include <stdio.h>
#include <stdlib.h>
#include "builtin.h"
#include "vm.h"

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

vm_object_t *builtin_constructor(vm_state *vm, vm_stack_t *args) {
  // return itself
  return args[-1].obj;
}

static int lastGetChar, getBits = 0;

vm_object_t *builtin_getchar(vm_state *vm, vm_stack_t *args) {
  if (getBits == 0) {
    getBits = 8;
    lastGetChar = getchar();
  }
  getBits--;
  if (lastGetChar == EOF) return NULL;
  return (lastGetChar>>getBits & 1) ? args[-1].obj : NULL;
}

vm_object_t *builtin_feof(vm_state *vm, vm_stack_t *args) {
  return feof(stdin) ? args[-1].obj : NULL;
}

vm_object_t *builtin_puts(vm_state *vm, vm_stack_t *args) {
  vm_object_t *obj = args[0].obj;
  if (obj[-1].classId & VM_STRING_LIT) {
    // string object
    extern struct SizedString *strLitTable;
    struct SizedString *str = &strLitTable[obj[-1].classId & ~VM_STRING_LIT];
    fwrite(str->str, 1, str->len, stdout);
  }
  else {
    // TODO print structure
  }
  return NULL;
}

static int lastPutChar = 0, putBits = 0;

vm_object_t *builtin_putchar(vm_state *vm, vm_stack_t *args) {
  if (args[0].obj == NULL) {
    lastPutChar = lastPutChar<<1;
  }
  else {
    lastPutChar = lastPutChar<<1 | 1;
  }
  putBits++;
  if (putBits == 8) {
    putBits = 0;
    putchar(lastPutChar);
    lastPutChar = 0;
  }
  return NULL;
}

void addBuiltinMethods() {
  struct Class *Void = getVoidClass();
  struct Method *m;
  ClassType sig[100];
  // void()
  m = addConstructor(Method_BUILTIN, Void, createArgType(0, sig));
  m->builtinFun = builtin_constructor;
  // void getchar()
  m = addMethod(Method_BUILTIN, Void, Void, "getchar", createArgType(0, sig));
  m->builtinFun = builtin_getchar;
  // void feof()
  m = addMethod(Method_BUILTIN, Void, Void, "feof", createArgType(0, sig));
  m->builtinFun = builtin_feof;
  // void puts(void str)
  sig[0] = Void;
  m = addMethod(Method_BUILTIN, Void, Void, "puts", createArgType(1, sig));
  m->builtinFun = builtin_puts;
  // void putchar(void bit)
  sig[0] = Void;
  m = addMethod(Method_BUILTIN, Void, Void, "putchar", createArgType(1, sig));
  m->builtinFun = builtin_putchar;
}
