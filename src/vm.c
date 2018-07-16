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
  o = vm->heapUsed + 2;
  if (o + cls->fieldCount >= vm->heapLimit) {
    // TODO garbage collection
    return NULL;
  }
  vm->heapUsed += cls->fieldCount + 2;
  o[-2].field = NULL;
  o[-1].classId = cls->id;
  for (i = 0; i < cls->fieldCount; i++) {
    o[i].field = NULL;
  }
  return o;
}

void startProgram(struct VM_State *vm) {
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
    printf("constructor main() not found\n");
    return;
  }
  if (entryMethod->flag & Method_BUILTIN) {
    printf("constructor main() not found\n");
    return;
  }
  // initialize stack
  vm->heapUsed = vm->heap;
  vm->stack->obj = allocateObject(entryClass, vm);
  vm->fp = vm->stack + 1;
  for (i = 0; i < entryMethod->localCount; i++) {
    vm->fp[i].obj = NULL;
  }
  vm->sp = vm->fp + entryMethod->localCount + 3;
  vm->sp[-3].sp = NULL;
  vm->sp[-2].sp = NULL;
  vm->sp[-1].sp = NULL;
  vm->pc = entryMethod->bytecode;
  runByteCode(vm);
}

void runByteCode(struct VM_State *vm) {
  unsigned char *pc = vm->pc;
  vm_stack_t *sp = vm->sp;
  vm_stack_t *fp = vm->fp;
  vm_object_t *obj, *self = fp[-1].obj;
  while (1) {
    switch (*pc) {
      case Instr_NOP:
        break;
      case Instr_POP:
        sp--;
        break;
      case Instr_DUP:
        sp->obj = sp[-1].obj;
        sp++;
        break;
      case Instr_CALL:
        printf("call %d\n", pc[1] | pc[2]<<8);
        pc += 2;
        break;
      case Instr_STR:
        obj = allocateObject(getVoidClass(), vm);
        obj[-1].classId = pc[1] | pc[2]<<8 | VM_STRING_LIT;
        sp->obj = obj;
        sp++;
        pc += 2;
        break;
      case Instr_CALLSPECIAL:
        printf("call %d, %d\n", pc[1] | pc[2]<<8, pc[3] | pc[4]<<8);
        pc += 4;
        break;
      case Instr_NEW:
        obj = allocateObject(classTable[pc[1] | pc[2]<<8], vm);
        sp->obj = obj;
        sp++;
        pc += 2;
        break;
      case Instr_THIS:
        sp->obj = self;
        sp++;
        break;
      case Instr_NULL:
        sp->obj = NULL;
        sp++;
        break;
      case Instr_LOAD:
        sp->obj = fp[pc[1] | pc[2]<<8].obj;
        sp++;
        pc += 2;
        break;
      case Instr_STORE:
        fp[pc[1] | pc[2]<<8].obj = sp[-1].obj;
        pc += 2;
        break;
      case Instr_PUTFIELD:
        self[pc[1] | pc[2]<<8].field = sp[-1].obj;
        pc += 2;
        break;
      case Instr_GETFIELD:
        sp->obj = self[pc[1] | pc[2]<<8].field;
        sp++;
        pc += 2;
        break;
      case Instr_CONVERT:
        obj = sp[-1].obj;
        if (!isKindOf(classTable[obj->classId], classTable[pc[1] | pc[2]<<8])) {
          sp[-1].obj = NULL;
        }
        pc += 2;
        break;
      case Instr_RETURN:
        puts("return");
        return;
      default:
        printf("unknown instruction %02X\n", *pc);
    }
    pc++;
  }
}
