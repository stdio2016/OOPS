#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "class.h"
#include "ArrayList.h"

static volatile int interrupted = 0;

void showBytecode(unsigned char *code) {
  while (*code != Instr_RETURN) {
    printf(" %.2X", *code);
    code++;
  }
  puts(" 0E");
}

void garbageCollect(struct VM_State *vm) {
  vm_stack_t *sp = vm->sp, *fp = vm->fp, *fp2 = vm->fp2;
  vm_object h;
  vm_object *head = &h+2, *tail = &h+2;
  vm_object *tmp;
  h.field = NULL;
  // step 1: known references
  // to collect main object, which is always at stack bottom
  tmp = vm->stack[0].obj;
  tmp[-1].classId += VM_MARKED;
  tail[-2].field = tmp;
  tmp[-2].field = NULL;
  tail = tmp;
  while (fp2 != NULL) {
    // fp to fp2, fp2+3 to sp
    vm_stack_t *p;
    for (p = fp; p != fp2; p++) {
      tmp = tail[-2].field;
      if (p->obj != NULL && !(p->obj[-1].classId & VM_MARKED)) {
        p->obj[-1].classId += VM_MARKED;
        tail[-2].field = p->obj;
        p->obj[-2].field = tmp;
        tail = p->obj;
      }
    }
    for (p = fp2+3; p != sp; p++) {
      tmp = tail[-2].field;
      if (p->obj != NULL && !(p->obj[-1].classId & VM_MARKED)) {
        p->obj[-1].classId += VM_MARKED;
        tail[-2].field = p->obj;
        p->obj[-2].field = tmp;
        tail = p->obj;
      }
    }
    sp = fp;
    fp = fp2[1].sp;
    fp2 = fp2[0].sp;
  }
  // step 2: mark reachable
  while (head != tail) {
    head = head[-2].field;
    if (head[-1].classId & VM_STRING_LIT) {
      // string, no need to collect
    }
    else {
      struct Class *cls = classTable[head[-1].classId & VM_CLASS_MASK];
      int n = cls->fieldCount, i;
      for (i = 0; i < n; i++) {
        vm_object *p = head[i].field;
        tmp = tail[-2].field;
        if (p != NULL && !(p[-1].classId & VM_MARKED)) {
          p[-1].classId += VM_MARKED;
          tail[-2].field = p;
          p[-2].field = tmp;
          tail = p;
        }
      }
    }
  }
  // step 3: compute new address
  head = vm->heap;
  vm_object *used = vm->heap;
  while (head != vm->heapUsed) {
    int n;
    if (head[1].classId & VM_STRING_LIT) {
      // string
      n = 2;
    }
    else {
      struct Class *cls = classTable[head[1].classId & VM_CLASS_MASK];
      n = 2 + cls->fieldCount;
    }
    if (head[1].classId & VM_MARKED) {
      head[0].field = used + 2; // remapped address
      used += n;
    }
    head += n;
  }
  // step 4: update references on stack
  sp = vm->sp; fp = vm->fp; fp2 = vm->fp2;
  while (fp2 != NULL) {
    // fp to fp2, fp2+3 to sp
    vm_stack_t *p;
    for (p = fp; p != fp2; p++) {
      if (p->obj != NULL) {
        p->obj = p->obj[-2].field;
      }
    }
    for (p = fp2+3; p != sp; p++) {
      if (p->obj != NULL) {
        p->obj = p->obj[-2].field;
      }
    }
    sp = fp;
    fp = fp2[1].sp;
    fp2 = fp2[0].sp;
  }
  // step 5: update references on heap
  head = vm->heap;
  while (head != vm->heapUsed) {
    int n, i;
    if (head[1].classId & VM_STRING_LIT) {
      // string
      n = 2;
    }
    else {
      struct Class *cls = classTable[head[1].classId & VM_CLASS_MASK];
      n = cls->fieldCount + 2;
      if (head[1].classId & VM_MARKED) {
        for (i = 0; i < cls->fieldCount; i++) {
          if (head[i+2].field != NULL) {
            head[i+2].field = head[i+2].field[-2].field;
          }
        }
      }
    }
    head += n;
  }
  // step 6: move object
  head = vm->heap;
  while (head != vm->heapUsed) {
    int n, i;
    if (head[1].classId & VM_STRING_LIT) {
      // string
      n = 2;
    }
    else {
      struct Class *cls = classTable[head[1].classId & VM_CLASS_MASK];
      n = cls->fieldCount + 2;
    }
    if (head[1].classId & VM_MARKED) {
      head[1].classId -= VM_MARKED;
      memmove(head[0].field - 2, head, n * sizeof(*head));
    }
    head += n;
  }
  vm->heapUsed = used;
  memset(used, 0, (vm->heapLimit - used) * sizeof(*head));
}

union VM_Object *allocateObject(struct Class *cls, struct VM_State *vm) {
  union VM_Object *o;
  int i;
  o = vm->heapUsed + 2;
  if (o + cls->fieldCount >= vm->heapLimit) {
    garbageCollect(vm);
    // try again
    o = vm->heapUsed + 2;
    if (o + cls->fieldCount >= vm->heapLimit) {
      return NULL;
    }
  }
  vm->heapUsed += cls->fieldCount + 2;
  o[-2].field = NULL;
  o[-1].classId = cls->id;
  for (i = 0; i < cls->fieldCount; i++) {
    o[i].field = NULL;
  }
  return o;
}

#define STACKCHECK() if (sp >= vm->stackLimit) { \
  vm->pc = pc; \
  return VM_RunResult_StackOverflow; \
}

int startProgram(struct VM_State *vm) {
  struct Class *entryClass = getClass("main");
  vm->sp = NULL;
  if (entryClass == NULL || !entryClass->defined) {
    return VM_RunResult_NoMainClass;
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
    return VM_RunResult_NoEntryPoint;
  }
  if (entryMethod->flag & Method_BUILTIN) {
    return VM_RunResult_NoEntryPoint;
  }
  // initialize stack
  vm->heapUsed = vm->heap;
  vm->fp = vm->stack + 1;
  vm->fp2 = vm->fp + entryMethod->localCount;
  vm->sp = vm->fp2 + 3;
  if (vm->sp >= vm->stackLimit) {
    vm->sp = NULL;
    return VM_RunResult_StackOverflow;
  }
  vm->stack->obj = allocateObject(entryClass, vm);
  for (i = 0; i < entryMethod->localCount; i++) {
    vm->fp[i].obj = NULL;
  }
  vm->sp[-3].sp = NULL;
  vm->sp[-2].sp = NULL;
  vm->sp[-1].ip = NULL;
  vm->pc = entryMethod->bytecode;
  return runByteCode(vm);
}

static void intr(int sig) {
  interrupted = 1;
}

int runByteCode(struct VM_State *vm) {
  interrupted = 0;
  signal(SIGINT, intr);
  struct Class *VoidClass = getVoidClass();
  unsigned char *pc = vm->pc;
  vm_stack_t *stack = vm->stack;
  vm_stack_t *sp = vm->sp;
  vm_stack_t *fp = vm->fp;
  vm_object *obj, *self = fp[-1].obj;
  struct Class *cls;
  struct Method *meth;
  int arity;
  int i;
  vm_stack_t *tmp = vm->fp;
  while (!interrupted) {
    //printf("%02X sp %d fp %d\n", *pc, sp - stack, fp - stack);
    switch (*pc) {
      case Instr_NOP:
        break;
      case Instr_POP:
        sp--;
        break;
      case Instr_DUP:
        STACKCHECK();
        sp->obj = sp[-1].obj;
        sp++;
        break;
      case Instr_CALL:
        //printf("call %d, %d\n", pc[1] | pc[2]<<8, pc[3] | pc[4]<<8);
        vm->fp = fp;
        vm->sp = sp;
        arity = pc[1] | pc[2]<<8;
        obj = (sp-arity-1)->obj;
        if (obj != NULL) {
          if (obj[-1].classId & VM_STRING_LIT) {
            cls = VoidClass;
          }
          else {
            cls = classTable[obj[-1].classId & VM_CLASS_MASK];
          }
          meth = cls->methodTable[pc[3] | pc[4]<<8];
        }
        if (obj == NULL || meth->flag & Method_BUILTIN) {
          sp = sp - arity;
          vm->pc = pc;
          if (obj == NULL) {
            sp[-1].obj = NULL;
          }
          else {
            sp[-1].obj = meth->builtinFun(vm, sp);
          }
          pc += 4;
        }
        else {
          // prepare stack space
          fp = sp - arity;
          sp = fp + meth->localCount + 3;
          STACKCHECK();
          sp[-3].sp = vm->fp2;
          sp[-2].sp = vm->fp;
          sp[-1].ip = pc + 4;
          vm->fp2 = fp + meth->localCount;
          pc = meth->bytecode - 1;
          for (i = arity; i < meth->localCount; i++) {
            fp[i].obj = NULL;
          }
          self = obj;
        }
        break;
      case Instr_STR:
        vm->sp = sp;
        vm->fp = fp;
        obj = allocateObject(getVoidClass(), vm);
        if (obj == NULL) return VM_RunResult_OOM;
        self = fp[-1].obj;
        obj[-1].classId = pc[1] | pc[2]<<8 | VM_STRING_LIT;
        STACKCHECK();
        sp->obj = obj;
        sp++;
        pc += 2;
        break;
      case Instr_CALLSPECIAL:
        //printf("callspecial %d, %d\n", pc[1] | pc[2]<<8, pc[3] | pc[4]<<8);
        vm->fp = fp;
        vm->sp = sp;
        cls = classTable[pc[1] | pc[2]<<8];
        meth = cls->methodTable[pc[3] | pc[4]<<8];
        arity = meth->args.arity;
        obj = (sp-arity-1)->obj;
        if (obj == NULL || meth->flag & Method_BUILTIN) {
          sp = sp - arity;
          vm->pc = pc;
          if (obj == NULL) {
            sp[-1].obj = NULL;
          }
          else {
            sp[-1].obj = meth->builtinFun(vm, sp);
          }
          pc += 4;
        }
        else {
          // prepare stack space
          fp = sp - arity;
          sp = fp + meth->localCount + 3;
          STACKCHECK();
          sp[-3].sp = vm->fp2;
          sp[-2].sp = vm->fp;
          sp[-1].ip = pc + 4;
          vm->fp2 = fp + meth->localCount;
          pc = meth->bytecode - 1;
          for (i = arity; i < meth->localCount; i++) {
            fp[i].obj = NULL;
          }
          self = obj;
        }
        break;
      case Instr_NEW:
        vm->sp = sp;
        vm->fp = fp;
        obj = allocateObject(classTable[pc[1] | pc[2]<<8], vm);
        if (obj == NULL) return VM_RunResult_OOM;
        self = fp[-1].obj;
        STACKCHECK();
        sp->obj = obj;
        sp++;
        pc += 2;
        break;
      case Instr_THIS:
        STACKCHECK();
        sp->obj = self;
        sp++;
        break;
      case Instr_NULL:
        STACKCHECK();
        sp->obj = NULL;
        sp++;
        break;
      case Instr_LOAD:
        STACKCHECK();
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
        STACKCHECK();
        sp->obj = self[pc[1] | pc[2]<<8].field;
        sp++;
        pc += 2;
        break;
      case Instr_CONVERT:
        obj = sp[-1].obj;
        if (obj != NULL) {
          if (obj[-1].classId & VM_STRING_LIT) {
            cls = VoidClass;
          }
          else {
            cls = classTable[obj[-1].classId & VM_CLASS_MASK];
          }
          if (!isKindOf(cls, classTable[pc[1] | pc[2]<<8])) {
            sp[-1].obj = NULL;
          }
        }
        pc += 2;
        break;
      case Instr_RETURN:
        obj = sp[-1].obj;
        tmp = fp;
        fp = sp[-3].sp;
        pc = sp[-2].ip;
        vm->fp2 = sp[-4].sp;
        sp = tmp;
        if (pc == 0) return VM_RunResult_Finish; // finish
        sp[-1].obj = obj;
        self = fp[-1].obj;
        break;
      default:
        return VM_RunResult_InternalError;
    }
    pc++;
  }
  vm->sp = sp;
  vm->fp = fp;
  vm->pc = pc;
  return VM_RunResult_Interrupt;
}

void stackTrace(struct VM_State *vm) {
  vm_stack_t *sp = vm->sp, *fp = vm->fp, *fp2 = vm->fp2;
  unsigned char *ip = vm->pc;
  if (vm->sp == NULL) return;
  while (fp2 != NULL) {
    // fp to fp2, fp2+3 to sp
    vm_stack_t *p = fp2+2;
    vm_object *obj = fp[-1].obj;
    int clsid = obj[-1].classId;
    if (clsid & VM_STRING_LIT) clsid = 0;
    else clsid = clsid & VM_CLASS_MASK;
    struct Class *cls = classTable[clsid];
    int i;
    // find possible method
    struct Method *maybe = NULL;
    for (i = 0; i < cls->methodCount; i++) {
      struct Method *meth = cls->methodTable[i];
      if (!(meth->flag & Method_BUILTIN) && meth->bytecode <= ip) {
        if (maybe == NULL || meth->bytecode > maybe->bytecode) {
          maybe = meth;
        }
      }
    }
    if (maybe == NULL) {
      printf("at %s.<unknown>\n", cls->name);
    }
    else {
      printf("at %s.%s + %zd\n", cls->name, maybe->name, ip - maybe->bytecode);
    }
    ip = p->ip;
    sp = fp;
    fp = fp2[1].sp;
    fp2 = fp2[0].sp;
  }
}
