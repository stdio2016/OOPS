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

vm_object *builtin_constructor(vm_state *vm, vm_stack_t *args) {
  // return itself
  return args[-1].obj;
}

static int lastGetChar, getBits = 0;

vm_object *builtin_getchar(vm_state *vm, vm_stack_t *args) {
  if (getBits == 0) {
    getBits = 8;
    lastGetChar = getchar();
  }
  getBits--;
  if (lastGetChar == EOF) return NULL;
  return (lastGetChar>>getBits & 1) ? args[-1].obj : NULL;
}

vm_object *builtin_feof(vm_state *vm, vm_stack_t *args) {
  return feof(stdin) ? args[-1].obj : NULL;
}

void printStruct(vm_object *obj, vm_object *heap) {
  vm_object *cur = obj, *ptr = obj;
  struct Class *c = classTable[obj[-1].classId & VM_CLASS_MASK];
  vm_object *end = obj + c->fieldCount;
  obj[-1].classId |= VM_MARKED;
  if (obj == end) {
    // object is empty
    printf("%s#%d {", c->name, obj - heap);
  }

  // print structure and mark visited
  while (ptr != end) {
    struct Class *cls = classTable[cur[-1].classId & VM_CLASS_MASK];
    if (cur == ptr) {
      printf("%s#%d {", cls->name, cur - heap);
    }

    // at end?
    if (ptr - cur >= cls->fieldCount) {
      vm_object *tmp = cur;
      ptr = tmp[-2].field;
      cur = ptr->field;
      ptr->field = tmp;
      tmp[-2].field = NULL;
      putchar('}');
      ptr++;
      continue;
    }
    else if (ptr > cur) printf(", ");

    vm_object *next = ptr->field;
    if (next == NULL) {
      printf("null");
      ptr++;
    }
    else if (next[-1].classId & VM_STRING_LIT) {
      // string object
      extern struct SizedString **strLitTable;
      struct SizedString *str = strLitTable[next[-1].classId & VM_CLASS_MASK];
      putchar('"');
      fwrite(str->str, 1, str->len, stdout);
      putchar('"');
      ptr++;
    }
    else if (next[-1].classId & VM_MARKED) {
      cls = classTable[next[-1].classId & VM_CLASS_MASK];
      printf("%s#%d", cls->name, next - heap);
      ptr++;
    }
    else {
      next[-1].classId |= VM_MARKED;
      ptr->field = cur;
      next[-2].field = ptr;
      cur = next;
      ptr = next;
    }
  }

  printf("}");

  // remove mark
  ptr = cur = obj;
  obj[-1].classId ^= VM_MARKED;
  while (ptr != end) {
    struct Class *cls = classTable[cur[-1].classId & VM_CLASS_MASK];

    // at end?
    if (ptr - cur >= cls->fieldCount) {
      vm_object *tmp = cur;
      ptr = tmp[-2].field;
      cur = ptr->field;
      ptr->field = tmp;
      tmp[-2].field = NULL;
      ptr++;
      continue;
    }

    vm_object *next = ptr->field;
    if (next == NULL) {
      ptr++;
    }
    else if (next[-1].classId & VM_STRING_LIT) {
      ptr++;
    }
    else if ((next[-1].classId & VM_MARKED) == 0) {
      ptr++;
    }
    else {
      next[-1].classId ^= VM_MARKED;
      ptr->field = cur;
      next[-2].field = ptr;
      cur = next;
      ptr = next;
    }
  }
}

vm_object *builtin_puts(vm_state *vm, vm_stack_t *args) {
  vm_object *obj = args[0].obj;
  if (obj == NULL) {
    printf("null");
  }
  else if (obj[-1].classId & VM_STRING_LIT) {
    // string object
    extern struct SizedString **strLitTable;
    struct SizedString *str = strLitTable[obj[-1].classId & ~VM_STRING_LIT];
    fwrite(str->str, 1, str->len, stdout);
  }
  else {
    printStruct(obj, vm->heap);
  }
  return NULL;
}

static int lastPutChar = 0, putBits = 0;

vm_object *builtin_putchar(vm_state *vm, vm_stack_t *args) {
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
