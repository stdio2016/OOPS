#include <stdio.h>
#include "vm.h"
#include "class.h"
#include "ArrayList.h"

void showBytecode(unsigned char *code) {
  while (*code != Instr_RETURN) {
    printf(" %.2X", *code);
    code++;
  }
  puts(" 0E");
}

union VM_Object *allocateObject(struct Class *cls, struct VM_State *vm) {
  union VM_Object *o;
  int i;
  o = vm->heapUsed;
  o[0].classId = cls->id;
  for (i = 1; i <= cls->fieldCount; i++) {
    o[i].field = NULL;
  }
  vm->heapUsed += cls->fieldCount + 1;
  return o;
}

void runProgram(struct VM_State *vm) {
  struct Class *entryClass = getClass("main");
  if (entryClass == NULL || !entryClass->defined) {
    printf("entry class \"main\" not found\n");
    return;
  }
  struct ArrayList *cons = MyHash_get(&entryClass->methods, "<init>");
  struct Method *entryMethod;
  int i;
  for (i = 0; i < cons->size; i++) {
    entryMethod = ArrayList_get(cons, i);
    if (entryMethod->args.arity == 0) {
      break;
    }
  }
  if (i == cons->size) {
    printf("default constructor of \"main\" not found\n");
    return;
  }
  // initialize stack
  vm->stack->obj = allocateObject(entryClass, vm);
  vm->fp = vm->stack + 1;
  for (i = 0; i < entryMethod->localCount; i++) {
    vm->fp[i].obj = NULL;
  }
  vm->sp = vm->fp + entryMethod->localCount + 3;
  vm->sp[-3].sp = NULL;
  vm->sp[-2].sp = NULL;
  vm->sp[-1].sp = NULL;
}
