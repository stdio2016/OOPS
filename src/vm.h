#pragma once
#ifndef VM_INCLUDED
#define VM_INCLUDED

enum VM_Bytecodes {
  Instr_NOP,
  Instr_POP,
  Instr_DUP,
  Instr_CALL,
  Instr_STR,
  Instr_CALLSPECIAL,
  Instr_NEW,
  Instr_THIS,
  Instr_NULL,
  Instr_LOAD,
  Instr_STORE,
  Instr_PUTFIELD,
  Instr_GETFIELD,
  Instr_CONVERT,
  Instr_RETURN
};

void showBytecode(unsigned char *code);

// stack structure
// 1. this
// 2. arguments <- fp
// 3. locals
// 4. stack pointer, frame pointer, return address
// 5. stack <- sp

union VM_Object {
  int classId;
  union VM_Object *field;
};

// flags for classId
#define VM_STRING_LIT 0x40000

union VM_StackType {
  union VM_Object *obj;
  unsigned char *ip;
  union VM_StackType *sp;
};

struct VM_State {
  union VM_StackType *sp, *fp;
  union VM_StackType *stack, *stackLimit;
  union VM_Object *heap, *heapLimit;
  union VM_Object *heapUsed;
  unsigned char *pc;
};

typedef union VM_StackType vm_stack_t;
typedef struct VM_State vm_state;
typedef union VM_Object vm_object_t;

void startProgram(struct VM_State *state);
void runByteCode(struct VM_State *state);
#endif
